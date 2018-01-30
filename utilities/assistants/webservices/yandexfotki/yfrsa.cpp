/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-14
 * Description : Yandex authentication module
 *
 * This is an Yandex implementation of RSA algorithm.
 * It differs from the standard RSA and incompatible with it.
 *
 * Based on code parts from pegwit program written by George Barwood.
 * This code is in the public domain; do with it what you wish.
 *
 * See links for more details:
 *  Author homepage
 *    http://www.george-barwood.pwp.blueyonder.co.uk/hp/
 *
 *  Getting token for Yandex.Fotki web service
 *    http://api.yandex.ru/fotki/doc/overview/authorization-token.xml
 *
 *  Yandex published source code
 *    http://download.yandex.ru/api-fotki/c-yamrsa.tar.gz
 *
 *  Yandex company web site
 *    http://company.yandex.com/
 *
 * Included by Roman Tsisyk <roman at tsisyk dot com>
 * All unneeded parts was commented out and can be removed
 *
 * ============================================================ */

#include "yfrsa.h"

// C++ includes

#include <cstdlib> // std::size_t
#include <cstring>

namespace YFAuth
{

static vlong modexp( const vlong& x, const vlong& e, const vlong& m );    // m must be odd
static vlong modinv( const vlong& a, const vlong& m );                    // modular inverse

// VLONG.CPP -----------------------------------

class flex_unit // Provides storage allocation and index checking
{
public:

    unsigned* a;  // array of units
    unsigned z; // units allocated

    unsigned n; // used units (read-only)
    flex_unit();
    ~flex_unit();
    void clear(); // set n to zero
    unsigned get( unsigned i ) const;       // get ith unsigned
    void set( unsigned i, unsigned x ); // set ith unsigned
    void reserve( unsigned x );         // storage hint

    // Time critical routine
    void fast_mul( flex_unit& x, flex_unit& y, unsigned n );
};

class vlong_value : public flex_unit
{
public:
    unsigned share; // share count, used by vlong to delay physical copying
    int is_zero() const;
    int test( unsigned i ) const;
    unsigned bits() const;
    int cf( vlong_value& x ) const;
    void shl();
    void shr();
    void shr( unsigned n );
    void add( vlong_value& x );
    void subtract( vlong_value& x );
    void init( unsigned x );
    void copy( vlong_value& x );
    operator unsigned(); // Unsafe conversion to unsigned
    vlong_value();
    void mul( vlong_value& x, vlong_value& y );
    void divide( vlong_value& x, vlong_value& y, vlong_value& rem );
};

unsigned flex_unit::get( unsigned i ) const
{
    if ( i >= n )
    {
        return 0;
    }

    return a[i];
}

void flex_unit::clear()
{
    n = 0;
}

flex_unit::flex_unit()
{
    z = 0;
    a = 0;
    n = 0;
}

flex_unit::~flex_unit()
{
    unsigned i=z;

    while (i)
    {
        i-=1;    // burn
        a[i] = 0;
    }

    delete [] a;
}

void flex_unit::reserve( unsigned x )
{
    if (x > z)
    {
        unsigned* na = new unsigned[x];

        for (unsigned i=0; i<n; i+=1)
        {
            na[i] = a[i];
        }

        delete [] a;
        a = na;
        z = x;
    }
}

void flex_unit::set( unsigned i, unsigned x )
{
    if ( i < n )
    {
        a[i] = x;

        if (x==0) while (n && a[n-1]==0)
            {
                n-=1;    // normalise
            }
    }
    else if ( x )
    {
        reserve(i+1);

        for (unsigned j=n; j<i; j+=1)
        {
            a[j] = 0;
        }

        a[i] = x;
        n = i+1;
    }
}

// Macros for doing double precision multiply
#define BPU ( 8*sizeof(unsigned) )       // Number of bits in an unsigned
#define lo(x) ( (x) & ((1<<(BPU/2))-1) ) // lower half of unsigned
#define hi(x) ( (x) >> (BPU/2) )         // upper half
#define lh(x) ( (x) << (BPU/2) )         // make upper half

void flex_unit::fast_mul( flex_unit& x, flex_unit& y, unsigned keep )
{
    // *this = (x*y) % (2**keep)
    unsigned i,j,limit = (keep+BPU-1)/BPU; // size of result in words
    reserve(limit);

    for (i=0; i<limit; i+=1)
    {
        a[i] = 0;
    }

    unsigned min = x.n;

    if (min>limit)
    {
        min = limit;
    }

    for (i=0; i<min; i+=1)
    {
        unsigned m = x.a[i];
        unsigned c = 0; // carry
        unsigned min = i+y.n;

        if (min>limit)
        {
            min = limit;
        }

        for ( j=i; j<min; j+=1 )
        {
            // This is the critical loop
            // Machine dependent code could help here
            // c:a[j] = a[j] + c + m*y.a[j-i];
            unsigned w, v = a[j], p = y.a[j-i];
            v += c;
            c = ( v < c );
            w = lo(p)*lo(m);
            v += w;
            c += ( v < w );
            w = lo(p)*hi(m);
            c += hi(w);
            w = lh(w);
            v += w;
            c += ( v < w );
            w = hi(p)*lo(m);
            c += hi(w);
            w = lh(w);
            v += w;
            c += ( v < w );
            c += hi(p) * hi(m);
            a[j] = v;
        }

        while ( c && j<limit )
        {
            a[j] += c;
            c = a[j] < c;
            j += 1;
        }
    }

    // eliminate unwanted bits
    keep %= BPU;

    if (keep)
    {
        a[limit-1] &= (1<<keep)-1;
    }

    // calculate n
    while (limit && a[limit-1]==0)
    {
        limit-=1;
    }

    n = limit;
}

vlong_value::operator unsigned()
{
    return get(0);
}

int vlong_value::is_zero() const
{
    return n==0;
}

int vlong_value::test( unsigned i ) const
{
    return ( get(i/BPU) & (1<<(i%BPU)) ) != 0;
}

unsigned vlong_value::bits() const
{
    unsigned x = n*BPU;

    while (x && test(x-1)==0)
    {
        x -= 1;
    }

    return x;
}

int vlong_value::cf( vlong_value& x ) const
{
    if ( n > x.n )
    {
        return +1;
    }

    if ( n < x.n )
    {
        return -1;
    }

    unsigned i = n;

    while (i)
    {
        i -= 1;

        if ( get(i) > x.get(i) )
        {
            return +1;
        }

        if ( get(i) < x.get(i) )
        {
            return -1;
        }
    }

    return 0;
}

void vlong_value::shl()
{
    unsigned carry = 0;
    unsigned N = n; // necessary, since n can change

    for (unsigned i=0; i<=N; i+=1)
    {
        unsigned u = get(i);
        set(i,(u<<1)+carry);
        carry = u>>(BPU-1);
    }
}

void vlong_value::shr()
{
    unsigned carry = 0;
    unsigned i=n;

    while (i)
    {
        i -= 1;
        unsigned u = get(i);
        set(i,(u>>1)+carry);
        carry = u<<(BPU-1);
    }
}

void vlong_value::shr( unsigned x )
{
    unsigned delta = x/BPU;
    x %= BPU;

    for (unsigned i=0; i<n; i+=1)
    {
        unsigned u = get(i+delta);

        if (x)
        {
            u >>= x;
            u += get(i+delta+1) << (BPU-x);
        }

        set(i,u);
    }
}

void vlong_value::add( vlong_value& x )
{
    unsigned carry = 0;
    unsigned max = n;

    if (max<x.n)
    {
        max = x.n;
    }

    reserve(max);

    for (unsigned i=0; i<max+1; i+=1)
    {
        unsigned u = get(i);
        u = u + carry;
        carry = ( u < carry );
        unsigned ux = x.get(i);
        u = u + ux;
        carry += ( u < ux );
        set(i,u);
    }
}

void vlong_value::subtract( vlong_value& x )
{
    unsigned carry = 0;
    unsigned N = n;

    for (unsigned i=0; i<N; i+=1)
    {
        unsigned ux = x.get(i);
        ux += carry;

        if ( ux >= carry )
        {
            unsigned u = get(i);
            unsigned nu = u - ux;
            carry = nu > u;
            set(i,nu);
        }
    }
}

void vlong_value::init( unsigned x )
{
    clear();
    set(0,x);
}

void vlong_value::copy( vlong_value& x )
{
    clear();
    unsigned i=x.n;

    while (i)
    {
        i -= 1;
        set( i, x.get(i) );
    }
}

vlong_value::vlong_value()
{
    share = 0;
}

void vlong_value::mul( vlong_value& x, vlong_value& y )
{
    fast_mul( x, y, x.bits()+y.bits() );
}

void vlong_value::divide( vlong_value& x, vlong_value& y, vlong_value& rem )
{
    init(0);
    rem.copy(x);
    vlong_value m,s;
    m.copy(y);
    s.init(1);

    while ( rem.cf(m) > 0 )
    {
        m.shl();
        s.shl();
    }

    while ( rem.cf(y) >= 0 )
    {
        while ( rem.cf(m) < 0 )
        {
            m.shr();
            s.shr();
        }

        rem.subtract( m );
        add( s );
    }
}

// Implementation of vlong

void vlong::load( unsigned* a, unsigned n )
{
    docopy();
    value->clear();

    for (unsigned i=0; i<n; i+=1)
    {
        value->set(i,a[i]);
    }
}

void vlong::store( unsigned* a, unsigned n ) const
{
    for (unsigned i=0; i<n; i+=1)
    {
        a[i] = value->get(i);
    }
}

unsigned vlong::get_nunits() const
{
    return value->n;
}

unsigned vlong::bits() const
{
    return value->bits();
}

void vlong::docopy()
{
    if ( value->share )
    {
        value->share -= 1;
        vlong_value* nv = new vlong_value;
        nv->copy(*value);
        value = nv;
    }
}

int vlong::cf( const vlong& x ) const
{
    int neg = negative && !value->is_zero();
    //int neg2 = x.negative && !x.value->is_zero();

    if ( neg == (x.negative && !x.value->is_zero()) )
        //if ( neg == neg2)
    {
        return value->cf( *x.value );
    }

    else if ( neg )
    {
        return -1;
    }
    else
    {
        return +1;
    }
}

vlong::vlong (unsigned x)
{
    value = new vlong_value;
    negative = 0;
    value->init(x);
}

vlong::vlong ( const vlong& x ) // copy constructor
{
    negative = x.negative;
    value = x.value;
    value->share += 1;
}

vlong& vlong::operator =(const vlong& x)
{
    if ( value->share )
    {
        value->share -=1;
    }
    else
    {
        delete value;
    }

    value = x.value;
    value->share += 1;
    negative = x.negative;
    return *this;
}

vlong::~vlong()
{
    if ( value->share )
    {
        value->share -=1;
    }
    else
    {
        delete value;
    }
}

vlong::operator unsigned () // conversion to unsigned
{
    return *value;
}

vlong& vlong::operator +=(const vlong& x)
{
    if ( negative == x.negative )
    {
        docopy();
        value->add( *x.value );
    }
    else if ( value->cf( *x.value ) >= 0 )
    {
        docopy();
        value->subtract( *x.value );
    }
    else
    {
        vlong tmp = *this;
        *this = x;
        *this += tmp;
    }

    return *this;
}

vlong& vlong::operator -=(const vlong& x)
{
    if ( negative != x.negative )
    {
        docopy();
        value->add( *x.value );
    }
    else if ( value->cf( *x.value ) >= 0 )
    {
        docopy();
        value->subtract( *x.value );
    }
    else
    {
        vlong tmp = *this;
        *this = x;
        *this -= tmp;
        negative = 1 - negative;
    }

    return *this;
}

vlong operator +( const vlong& x, const vlong& y )
{
    vlong result = x;
    result += y;
    return result;
}

vlong operator -( const vlong& x, const vlong& y )
{
    vlong result = x;
    result -= y;
    return result;
}

vlong operator *( const vlong& x, const vlong& y )
{
    vlong result;
    result.value->mul( *x.value, *y.value );
    result.negative = x.negative ^ y.negative;
    return result;
}

vlong operator /( const vlong& x, const vlong& y )
{
    vlong result;
    vlong_value rem;
    result.value->divide( *x.value, *y.value, rem );
    result.negative = x.negative ^ y.negative;
    return result;
}

#if defined(__DEBUG__)
void print_vlong( const vlong_value& v, const char* name )
{
    printf("%s value(%d): ", name, v.n * sizeof(unsigned int));

    for (int i = 0; i < v.n; ++i)
    {
        printf("%08X", v.a[i]);
    }

    printf("\n");
}
#endif

vlong operator %( const vlong& x, const vlong& y )
{
    vlong result;
    vlong_value divide;
    divide.divide( *x.value, *y.value, *result.value );
    result.negative = x.negative; // not sure about this?
    return result;
}
/*
static vlong gcd( const vlong &X, const vlong &Y )
{
    vlong x=X, y=Y;
    while (1)
    {
        if ( y == (vlong)0 ) return x;
        x = x % y;
        if ( x == (vlong)0 ) return y;
        y = y % x;
    }
}
*/

static vlong modinv( const vlong& a, const vlong& m ) // modular inverse
// returns i in range 1..m-1 such that i*a = 1 mod m
// a must be in range 1..m-1
{
    vlong j=1,i=0,b=m,c=a,x,y;

    while ( c != (vlong)0 )
    {
        x = b / c;
        y = b - x*c;
        b = c;
        c = y;
        y = j;
        j = i - j*x;
        i = y;
    }

    if ( i < (vlong)0 )
    {
        i += m;
    }

    return i;
}

class monty // class for montgomery modular exponentiation
{
    vlong R,R1,m,n1;
    vlong T,k;   // work registers
    unsigned N;  // bits for R
    void mul( vlong& x, const vlong& y );
public:
    vlong exp( const vlong& x, const vlong& e );
    monty( const vlong& M );
};

monty::monty( const vlong& M )
{
    m = M;
    N = 0;
    R = 1;

    while ( R < M )
    {
        R += R;
        N += 1;
    }

    R1 = modinv( R-m, m );
    n1 = R - modinv( m, R );
}

void monty::mul( vlong& x, const vlong& y )
{
    // T = x*y;
    T.value->fast_mul( *x.value, *y.value, N*2 );

    // k = ( T * n1 ) % R;
    k.value->fast_mul( *T.value, *n1.value, N );

    // x = ( T + k*m ) / R;
    x.value->fast_mul( *k.value, *m.value, N*2 );
    x += T;
    x.value->shr( N );

    if (x>=m)
    {
        x -= m;
    }
}

vlong monty::exp( const vlong& x, const vlong& e )
{
    vlong result = R-m, t = ( x * R ) % m;
    unsigned bits = e.value->bits();
    unsigned i = 0;

    while (1)
    {
        if ( e.value->test(i) )
        {
            mul( result, t);
        }

        i += 1;

        if ( i == bits )
        {
            break;
        }

        mul( t, t );
    }

    return ( result * R1 ) % m;
}

static vlong modexp( const vlong& x, const vlong& e, const vlong& m )
{
    monty me(m);
    return me.exp( x,e );
}

// RSA.CPP -----------------------------------

vlong public_key::encrypt( const vlong& plain )
{
#if defined(__DEBUG__)

    if ( plain >= m )
    {
        printf("ERROR: plain too big for this key\n");
    }

#endif
    return modexp( plain, e, m );
}

void str_2_vlong_pair (const char* me_str,vlong& m,vlong& e)
{
    int i;
    int dash_pos = 0;
    m = 0;
    e = 0;

    int me_len = (int)strlen (me_str);

    for (i = me_len-1; i>0; --i)
        if (me_str[i] == '#')
        {
            dash_pos = i;
            break;
        }

    if (dash_pos == 0)
    {
        return;
    }


    for (i = 0; i<dash_pos; ++i)
    {
        m = m * (vlong)16;

        if (me_str[i] > '9')
        {
            m = m + (vlong) (me_str[i]-'A'+10);
        }
        else
        {
            m = m + (vlong) (me_str[i]-'0');
        }
    }

    for (i = dash_pos+1; i<me_len; ++i)
    {
        e = e * (vlong)16;

        if (me_str[i] > '9')
        {
            e = e + (vlong) (me_str[i]-'A'+10);
        }
        else
        {
            e = e + (vlong) (me_str[i]-'0');
        }
    }


}

void public_key::MakeMe(const char* me_str)
{
    str_2_vlong_pair (me_str,m,e);
}

CCryptoProviderRSA::CCryptoProviderRSA()
{
}

CCryptoProviderRSA::~CCryptoProviderRSA()
{
}

void inline _rmemcpy (char* dst,const char* src,size_t size)
{
    src += size;

    while (size--)
    {
        *dst++ = *(--src);
    }
}

void CCryptoProviderRSA::EncryptPortion(const char* pt, size_t pt_size, char* ct, size_t& ct_size)
{
    vlong plain, cipher;

    const size_t bytes_per_unit = BPU / 8;

    size_t padding = (pt_size & 3) ? (4 - (pt_size & 3)) : 0;
    char tmp[MAX_CRYPT_BITS/4];

    // ensure big-endianness
    _rmemcpy(tmp, pt, pt_size);
    memset(tmp + pt_size, 0, padding);
    plain.load(reinterpret_cast<unsigned int*>(tmp), (int)(pt_size+padding) / bytes_per_unit);

    cipher = prkface.encrypt(plain);
    ct_size = cipher.get_nunits() * bytes_per_unit;

    // ensure big-endianness
    cipher.store(reinterpret_cast<unsigned int*>(tmp), (int)ct_size / bytes_per_unit);
    _rmemcpy(ct, tmp, ct_size);
}

void CCryptoProviderRSA::ImportPublicKey(const char* pk)
{
    prkface.MakeMe(pk);
}

void CCryptoProviderRSA::Encrypt(const char* inbuf, size_t in_size,char* outbuf, size_t& out_size)
{
    size_t i,cp_size;

    char portbuf[MAX_CRYPT_BITS/8];
    char cpbuf[MAX_CRYPT_BITS/4];
    const char* inp = inbuf;

    unsigned short lm;

    // must ensure that any data block would be < key's modulus
    // hence -1
    int portion_len = (prkface.m.bits() - 1)  / 8;

    if (portion_len < 0) portion_len = 0;

    char* prev_crypted = new char[portion_len];
    memset(prev_crypted, 0, portion_len);

    out_size = 0;

    while (in_size)
    {
        size_t uportion_len = (std::size_t) (portion_len);
        size_t cur_size     = in_size > uportion_len ? uportion_len : in_size;

        for (i=0; i<cur_size; ++i)
        {
            portbuf[i] = inp[i] ^ prev_crypted[i];
        }

        EncryptPortion(portbuf, cur_size, cpbuf, cp_size);

        for (i=0; i< uportion_len; ++i)
        {
            prev_crypted[i] = i < cp_size ? cpbuf[i] : 0;
        }

        lm=cur_size;
        memcpy (outbuf+out_size,&lm, sizeof(unsigned short));
        out_size+=sizeof (unsigned short);
        lm=(unsigned short)cp_size;
        memcpy (outbuf+out_size,&lm, sizeof(unsigned short));
        out_size+=sizeof (unsigned short);
        memcpy (outbuf+out_size,cpbuf, cp_size);
        out_size+=cp_size;
        inp+=cur_size;
        in_size-=cur_size;
    }

    delete [] prev_crypted;

    return;
}

} // namespace YFAuth
