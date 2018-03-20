/*
 * i-scream libstatgrab
 * http://www.i-scream.org
 * Copyright (C) 2000-2004 i-scream
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA
 *
 * $Id: memory_stats.c,v 1.36 2010/02/21 10:04:26 tdb Exp $
 */

/** Value returned : -1 : unsupported platform
 *                    0 : parse failure from supported platform
 *                    1 : parse done with sucess from supported platform
 */
static int get_mem_stats(Digikam::KMemoryInfo::KMemoryInfoData* const data);
static int get_swap_stats(Digikam::KMemoryInfo::KMemoryInfoData* const data);
static int fillMemoryInfo(Digikam::KMemoryInfo::KMemoryInfoData* const data)
{
    int ret = get_mem_stats(data);

    if (ret < 1)
    {
        data->valid = ret;
        return ret;
    }

    ret = get_swap_stats(data);

    if (ret < 1)
    {
        data->valid = ret;
        return ret;
    }

    data->valid = 1;

    return 1;
}


#ifdef Q_OS_SOLARIS
#include <unistd.h>
#include <kstat.h>
#endif
#if defined(Q_OS_LINUX)// || defined(Q_OS_CYGWIN)
#include <stdio.h>
#include <string.h>
#endif
#if defined(Q_OS_FREEBSD)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <kvm.h>
#include <paths.h>
#endif
#if defined(Q_OS_NETBSD)
#include <sys/param.h>
#include <sys/time.h>
#include <uvm/uvm.h>
#endif
#if defined(Q_OS_OPENBSD)
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/unistd.h>
#endif
#ifdef Q_OS_HPUX
#include <sys/param.h>
#include <sys/pstat.h>
#include <unistd.h>
#endif
#ifdef Q_OS_WIN
#include <windows.h>
#endif


#if defined(Q_OS_LINUX)
char* sg_f_read_line(FILE* f, const char* string)
{
    /* Max line length. 8k should be more than enough */
    static char line[8192];

    while((fgets(line, sizeof(line), f))!=NULL)
    {
        if (strncmp(string, line, strlen(string))==0)
        {
            return line;
        }
    }

    //sg_set_error(SG_ERROR_PARSE, NULL);
    return NULL;
}
#endif // defined(Q_OS_LINUX)

#if (defined(Q_OS_FREEBSD) && !defined(FREEBSD5)) || defined(DFBSD)
kvm_t* sg_get_kvm()
{
    static kvm_t* kvmd = NULL;

    if (kvmd != NULL)
    {
        return kvmd;
    }

    kvmd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, NULL);

    if (kvmd == NULL)
    {
        //sg_set_error(SG_ERROR_KVM_OPENFILES, NULL);
    }

    return kvmd;
}

/* Can't think of a better name for this function */
kvm_t* sg_get_kvm2()
{
    static kvm_t* kvmd2 = NULL;

    if (kvmd2 != NULL)
    {
        return kvmd2;
    }

    kvmd2 = kvm_openfiles(_PATH_DEVNULL, _PATH_DEVNULL, NULL, O_RDONLY, NULL);

    if (kvmd2 == NULL)
    {
        //sg_set_error(SG_ERROR_KVM_OPENFILES, NULL);
    }

    return kvmd2;
}
#endif // (defined(Q_OS_FREEBSD) && !defined(FREEBSD5)) || defined(DFBSD)

#if defined(Q_OS_NETBSD) || defined(Q_OS_OPENBSD)
struct uvmexp* sg_get_uvmexp()
{
    int                  mib[2];
    size_t               size = sizeof(struct uvmexp);
    static struct uvmexp uvm;
//    struct uvmexp*       new;

    mib[0] = CTL_VM;
    mib[1] = VM_UVMEXP;

    if (sysctl(mib, 2, &uvm, &size, NULL, 0) < 0)
    {
        //sg_set_error_with_errno(SG_ERROR_SYSCTL, "CTL_VM.VM_UVMEXP");
        return NULL;
    }

    return &uvm;
}
#endif // defined(Q_OS_NETBSD) || defined(Q_OS_OPENBSD)

#ifdef Q_OS_HPUX
struct pst_KMemoryInfo::static* sg_get_pstat_static()
{
    static int got = 0;
    static struct pst_static pst;

    if (!got)
    {
        if (pstat_getstatic(&pst, sizeof pst, 1, 0) == -1)
        {
            //sg_set_error_with_errno(SG_ERROR_PSTAT, "pstat_static");
            return NULL;
        }

        got = 1;
    }

    return &pst;
}
#endif // Q_OS_HPUX

// ----------------------------------------------------------------------------

int get_mem_stats(Digikam::KMemoryInfo::KMemoryInfoData* const data)
{

#ifdef Q_OS_HPUX
    struct pst_static* pstat_static = 0;
    struct pst_dynamic pstat_dynamic;
    long long          pagesize;
#endif // Q_OS_HPUX

#ifdef Q_OS_SOLARIS
    kstat_ctl_t*   kc  = 0;
    kstat_t*       ksp = 0;
    kstat_named_t* kn  = 0;
    long           totalmem;
    int            pagesize;
#endif // Q_OS_SOLARIS

#if defined(Q_OS_LINUX) || defined(Q_OS_CYGWIN)
    char*              line_ptr = 0;
    unsigned long long value;
    FILE*              f        = 0;
#endif // defined(Q_OS_LINUX) || defined(Q_OS_CYGWIN)

#if defined(Q_OS_FREEBSD) || defined(Q_OS_DFBSD)
    int    mib[2];
    u_long physmem;
    size_t size;
    u_long free_count;
    u_long cache_count;
    u_long inactive_count;
    int    pagesize;
#endif // defined(Q_OS_FREEBSD) || defined(Q_OS_DFBSD)

#if defined(Q_OS_NETBSD)
    struct uvmexp* uvm = 0;
#endif // defined(Q_OS_NETBSD)

#if defined(Q_OS_OPENBSD)
    int    mib[2];
    struct vmtotal vmtotal;
    size_t size;
    int    pagesize, page_multiplier;
#endif // defined(Q_OS_OPENBSD)

#ifdef Q_OS_WIN
    MEMORYSTATUSEX memstats;
#endif

#ifdef Q_OS_OSX
    Q_UNUSED(data);
#endif

#ifdef Q_OS_HPUX
    data->platform = QLatin1String("HPUX");

    if((pagesize = sysconf(_SC_PAGESIZE)) == -1)
    {
        //sg_set_error_with_errno(SG_ERROR_SYSCONF, "_SC_PAGESIZE");
        return 0;
    }

    if (pstat_getdynamic(&pstat_dynamic, sizeof(pstat_dynamic), 1, 0) == -1)
    {
        //sg_set_error_with_errno(SG_ERROR_PSTAT, "pstat_dynamic");
        return 0;
    }

    pstat_static = sg_get_pstat_static();

    if (pstat_static == NULL)
    {
        return 0;
    }

    /* FIXME Does this include swap? */
    data->totalRam = ((long long) pstat_static->physical_memory) * pagesize;
    data->freeRam  = ((long long) pstat_dynamic.psd_free)        * pagesize;
    data->usedRam  = data->totalRam - data->freeRam;

    return 1;
#endif // Q_OS_HPUX

#ifdef Q_OS_SOLARIS
    data->platform = QLatin1String("SOLARIS");

    if((pagesize = sysconf(_SC_PAGESIZE)) == -1)
    {
        //sg_set_error_with_errno(SG_ERROR_SYSCONF, "_SC_PAGESIZE");
        return 0;
    }

    if((totalmem = sysconf(_SC_PHYS_PAGES)) == -1)
    {
        //sg_set_error_with_errno(SG_ERROR_SYSCONF, "_SC_PHYS_PAGES");
        return 0;
    }

    if ((kc = kstat_open()) == NULL)
    {
        //sg_set_error(SG_ERROR_KSTAT_OPEN, NULL);
        return 0;
    }

    if((ksp = kstat_lookup(kc, "unix", 0, "system_pages")) == NULL)
    {
        //sg_set_error(SG_ERROR_KSTAT_LOOKUP, "unix,0,system_pages");
        return 0;
    }

    if (kstat_read(kc, ksp, 0) == -1)
    {
        //sg_set_error(SG_ERROR_KSTAT_READ, NULL);
        return 0;
    }

    if((kn = (kstat_named_t*)kstat_data_lookup(ksp, "freemem")) == NULL)
    {
        //sg_set_error(SG_ERROR_KSTAT_DATA_LOOKUP, "freemem");
        return 0;
    }

    kstat_close(kc);

    data->totalRam = (long long)totalmem * (long long)pagesize;
    data->freeRam  = ((long long)kn->value.ul) * (long long)pagesize;
    data->usedRam  = data->totalRam - data->freeRam;

    return 1;
#endif // Q_OS_SOLARIS

#if defined(Q_OS_LINUX) || defined(Q_OS_CYGWIN)
    data->platform = QLatin1String("LINUX");

    if ((f = fopen("/proc/meminfo", "r")) == NULL)
    {
        //sg_set_error_with_errno(SG_ERROR_OPEN, "/proc/meminfo");
        return 0;
    }

    while ((line_ptr = sg_f_read_line(f, "")) != NULL)
    {
        if (sscanf(line_ptr, "%*s %llu kB", &value) != 1)
        {
            continue;
        }

        value *= 1024;

        if (strncmp(line_ptr, "MemTotal:", 9) == 0)
        {
            data->totalRam = value;
        }
        else if (strncmp(line_ptr, "MemFree:", 8) == 0)
        {
            data->freeRam = value;
        }
        else if (strncmp(line_ptr, "Cached:", 7) == 0)
        {
            data->cacheRam = value;
        }
    }

    fclose(f);
    data->usedRam = data->totalRam - data->freeRam;

    return 1;
#endif // defined(Q_OS_LINUX) || defined(Q_OS_CYGWIN)

#if defined(Q_OS_FREEBSD)
    data->platform = QLatin1String("FREEBSD");

    /* Returns bytes */
    mib[0] = CTL_HW;
    mib[1] = HW_PHYSMEM;
    size   = sizeof physmem;

    if (sysctl(mib, 2, &physmem, &size, NULL, 0) < 0)
    {
        //sg_set_error_with_errno(SG_ERROR_SYSCTL, "CTL_HW.HW_PHYSMEM");
        return 0;
    }

    data->totalRam = physmem;

    /*returns pages*/
    size           = sizeof free_count;

    if (sysctlbyname("vm.stats.vm.v_free_count", &free_count, &size, NULL, 0) < 0)
    {
        //sg_set_error_with_errno(SG_ERROR_SYSCTLBYNAME, "vm.stats.vm.v_free_count");
        return 0;
    }

    size = sizeof inactive_count;

    if (sysctlbyname("vm.stats.vm.v_inactive_count", &inactive_count , &size, NULL, 0) < 0)
    {
        //sg_set_error_with_errno(SG_ERROR_SYSCTLBYNAME, "vm.stats.vm.v_inactive_count");
        return 0;
    }

    size = sizeof cache_count;

    if (sysctlbyname("vm.stats.vm.v_cache_count", &cache_count, &size, NULL, 0) < 0)
    {
        //sg_set_error_with_errno(SG_ERROR_SYSCTLBYNAME, "vm.stats.vm.v_cache_count");
        return 0;
    }

    /* Because all the vm.stats returns pages, I need to get the page size.
     * After that I then need to multiple the anything that used vm.stats to
     * get the system statistics by pagesize
     */
    pagesize       = getpagesize();
    data->cacheRam = cache_count * pagesize;

    /* Of couse nothing is ever that simple :) And I have inactive pages to
     * deal with too. So I'm going to add them to free memory :)
     */
    data->freeRam  = (free_count*pagesize)+(inactive_count*pagesize);
    data->usedRam  = physmem-data->freeRam;

    return 1;
#endif // defined(Q_OS_FREEBSD)

#if defined(Q_OS_NETBSD)
    data->platform = QLatin1String("NETBSD");

    if ((uvm = sg_get_uvmexp()) == NULL)
    {
        return 0;
    }

    data->totalRam = uvm->pagesize * uvm->npages;
    data->cacheRam = uvm->pagesize * (uvm->filepages + uvm->execpages);
    data->freeRam  = uvm->pagesize * (uvm->free + uvm->inactive);
    data->usedRam  = data->totalRam - data->freeRam;

    return 1;
#endif // defined(Q_OS_NETBSD)

#if defined(Q_OS_OPENBSD)
    data->platform = QLatin1String("OPENBSD");

    /* The code in this section is based on the code in the OpenBSD
     * top utility, located at src/usr.bin/top/machine.c in the
     * OpenBSD source tree.
     *
     * For fun, and like OpenBSD top, we will do the multiplication
     * converting the memory stats in pages to bytes in base 2.
     */

    /* All memory stats in OpenBSD are returned as the number of pages.
     * To convert this into the number of bytes we need to know the
     * page size on this system.
     */
    pagesize = sysconf(_SC_PAGESIZE);

    /* The pagesize gives us the base 10 multiplier, so we need to work
     * out what the base 2 multiplier is. This means dividing
     * pagesize by 2 until we reach unity, and counting the number of
     * divisions required.
     */
    page_multiplier = 0;

    while (pagesize > 1)
    {
        page_multiplier++;
        pagesize >>= 1;
    }

    /* We can now ret the raw VM stats (in pages) using the
     * sysctl interface.
     */
    mib[0] = CTL_VM;
    mib[1] = VM_METER;
    size   = sizeof(vmtotal);

    if (sysctl(mib, 2, &vmtotal, &size, NULL, 0) < 0)
    {
        bzero(&vmtotal, sizeof(vmtotal));
        //sg_set_error_with_errno(SG_ERROR_SYSCTL, "CTL_VM.VM_METER");
        return 0;
    }

    /* Convert the raw stats to bytes, and return these to the caller
     */
    data->usedRam  = (vmtotal.t_rm << page_multiplier);   /* total real mem in use */
    data->cacheRam = 0;                                  /* no cache stats */
    data->freeRam  = (vmtotal.t_free << page_multiplier); /* free memory pages */
    data->totalRam = (data->usedRam + data->freeRam);

    return 1;
#endif // defined(Q_OS_OPENBSD)

#ifdef Q_OS_WIN
    data->platform = QLatin1String("WINDOWS");

    memstats.dwLength = sizeof(memstats);

    if (!GlobalMemoryStatusEx(&memstats))
    {
        //sg_set_error_with_errno(SG_ERROR_MEMSTATUS, NULL);
        return 0;
    }

    data->freeRam  = memstats.ullAvailPhys;
    data->totalRam = memstats.ullTotalPhys;
    data->usedRam  = data->totalRam - data->freeRam;

    //if(read_counter_large(SG_WIN32_MEM_CACHE, &data->cacheRam)) {
        data->cacheRam = 0;
    //}

    return 1;
#endif // Q_OS_WIN

    return -1;
}

// ----------------------------------------------------------------------------

#ifdef Q_OS_SOLARIS
#ifdef _FILE_OFFSET_BITS
#undef _FILE_OFFSET_BITS
#endif
#include <sys/stat.h>
#include <sys/swap.h>
#include <unistd.h>
#endif
#if defined(Q_OS_LINUX) //|| defined(Q_OS_CYGWIN)
#include <stdio.h>
#include <string.h>
#endif
#if defined(Q_OS_FREEBSD)
#ifdef Q_OS_FREEBSD5
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#else
#include <sys/types.h>
#include <kvm.h>
#endif
#include <unistd.h>
#endif
#if defined(Q_OS_NETBSD) || defined(Q_OS_OPENBSD)
#include <sys/param.h>
#include <sys/time.h>
#include <uvm/uvm.h>
#include <unistd.h>
#endif
#ifdef Q_OS_HPUX
#include <sys/param.h>
#include <sys/pstat.h>
#include <unistd.h>
#define SWAP_BATCH 5
#endif
#ifdef Q_OS_WIN
#include <windows.h>
#endif

int get_swap_stats(Digikam::KMemoryInfo::KMemoryInfoData* const data)
{

#ifdef Q_OS_OSX
    Q_UNUSED(data);
#endif

#ifdef Q_OS_HPUX
    struct pst_swapinfo pstat_swapinfo[SWAP_BATCH];
    int                 swapidx = 0;
    int                 num, i;
#endif // Q_OS_HPUX

#ifdef Q_OS_SOLARIS
    struct anoninfo ai;
    int             pagesize;
#endif // Q_OS_SOLARIS

#if defined(Q_OS_LINUX) //|| defined(Q_OS_CYGWIN)
    FILE*              f        = 0;
    char*              line_ptr = 0;
    unsigned long long value;
#endif // defined(Q_OS_LINUX)

#if defined(Q_OS_FREEBSD)
    int pagesize;
#ifdef Q_OS_FREEBSD5
    struct xswdev xsw;
    int           mib[16], n;
    size_t        mibsize, size;
#else
    struct kvm_swap swapinfo;
    kvm_t*          kvmd = 0;
#endif // Q_OS_FREEBSD5

#endif // defined(Q_OS_FREEBSD)

#if defined(Q_OS_NETBSD) || defined(Q_OS_OPENBSD)
    struct uvmexp* uvm = 0;
#endif // defined(Q_OS_NETBSD) || defined(Q_OS_OPENBSD)

#ifdef Q_OS_WIN
    MEMORYSTATUSEX memstats;
#endif // Q_OS_WIN

#ifdef Q_OS_HPUX
    data->totalSwap = 0;
    data->usedSwap  = 0;
    data->freeSwap  = 0;

    while (1)
    {
        num = pstat_getswap(pstat_swapinfo, sizeof pstat_swapinfo[0], SWAP_BATCH, swapidx);

        if (num == -1)
        {
            //sg_set_error_with_errno(SG_ERROR_PSTAT,"pstat_getswap");
            return 0;
        }
        else if (num == 0)
        {
            break;
        }

        for (i = 0; i < num; i++)
        {
            struct pst_swapinfo* si = &pstat_swapinfo[i];

            if ((si->pss_flags & SW_ENABLED) != SW_ENABLED)
            {
                continue;
            }

            if ((si->pss_flags & SW_BLOCK) == SW_BLOCK)
            {
                data->totalSwap += ((long long) si->pss_nblksavail) * 1024LL;
                data->usedSwap  += ((long long) si->pss_nfpgs)      * 1024LL;
                data->freeSwap   = data->totalSwap - data->usedSwap;
            }

            if ((si->pss_flags & SW_FS) == SW_FS)
            {
                data->totalSwap += ((long long) si->pss_limit)     * 1024LL;
                data->usedSwap  += ((long long) si->pss_allocated) * 1024LL;
                data->freeSwap   = data->totalSwap - data->usedSwap;
            }
        }

        swapidx = pstat_swapinfo[num - 1].pss_idx + 1;
    }

    return 1;
#endif // Q_OS_HPUX

#ifdef Q_OS_SOLARIS
    if((pagesize=sysconf(_SC_PAGESIZE)) == -1)
    {
        //sg_set_error_with_errno(SG_ERROR_SYSCONF, "_SC_PAGESIZE");
        return 0;
    }

    if (swapctl(SC_AINFO, &ai) == -1)
    {
        //sg_set_error_with_errno(SG_ERROR_SWAPCTL, NULL);
        return 0;
    }

    data->totalSwap = (long long)ai.ani_max  * (long long)pagesize;
    data->usedSwap  = (long long)ai.ani_resv * (long long)pagesize;
    data->freeSwap  = data->totalSwap - data->usedSwap;

    return 1;
#endif // Q_OS_SOLARIS

#if defined(Q_OS_LINUX) || defined(Q_OS_CYGWIN)
    if ((f = fopen("/proc/meminfo", "r")) == NULL)
    {
        //sg_set_error_with_errno(SG_ERROR_OPEN, "/proc/meminfo");
        return 0;
    }

    while ((line_ptr = sg_f_read_line(f, "")) != NULL)
    {
        if (sscanf(line_ptr, "%*s %llu kB", &value) != 1)
        {
            continue;
        }

        value *= 1024;

        if (strncmp(line_ptr, "SwapTotal:", 10) == 0)
        {
            data->totalSwap = value;
        }
        else if (strncmp(line_ptr, "SwapFree:", 9) == 0)
        {
            data->freeSwap = value;
        }
    }

    fclose(f);
    data->usedSwap = data->totalSwap - data->freeSwap;

    return 1;
#endif // defined(Q_OS_LINUX) || defined(Q_OS_CYGWIN)

#if defined(Q_OS_FREEBSD) || defined(Q_OS_DFBSD)
    pagesize = getpagesize();

#ifdef Q_OS_FREEBSD5
    data->totalSwap = 0;
    data->usedSwap  = 0;

    mibsize = sizeof mib / sizeof mib[0];

    if (sysctlnametomib("vm.swap_info", mib, &mibsize) < 0)
    {
        //sg_set_error_with_errno(SG_ERROR_SYSCTLNAMETOMIB, "vm.swap_info");
        return 0;
    }

    for (n = 0; ; ++n)
    {
        mib[mibsize] = n;
        size         = sizeof xsw;

        if (sysctl(mib, mibsize + 1, &xsw, &size, NULL, 0) < 0)
        {
            break;
        }

        if (xsw.xsw_version != XSWDEV_VERSION)
        {
            //sg_set_error(SG_ERROR_XSW_VER_MISMATCH, NULL);
            return 0;
        }

        data->totalSwap += (long long) xsw.xsw_nblks;
        data->usedSwap  += (long long) xsw.xsw_used;
    }

    if (errno != ENOENT)
    {
        //sg_set_error_with_errno(SG_ERROR_SYSCTL, "vm.swap_info");
        return 0;
    }
#else // Q_OS_FREEBSD5
    if((kvmd = sg_get_kvm()) == NULL)
    {
        return 0;
    }

    if ((kvm_getswapinfo(kvmd, &swapinfo, 1,0)) == -1)
    {
        //sg_set_error(SG_ERROR_KVM_GETSWAPINFO, NULL);
        return 0;
    }

    data->totalSwap = (long long)swapinfo.ksw_total;
    data->usedSwap  = (long long)swapinfo.ksw_used;
#endif // Q_OS_FREEBSD5
    data->totalSwap *= pagesize;
    data->usedSwap  *= pagesize;
    data->freeSwap   = data->totalSwap - data->usedSwap;

    return 1;
#endif // defined(Q_OS_FREEBSD) || defined(Q_OS_DFBSD)

#if defined(Q_OS_NETBSD) || defined(Q_OS_OPENBSD)
    if ((uvm = sg_get_uvmexp()) == NULL)
    {
        return 0;
    }

    data->totalSwap = (long long)uvm->pagesize * (long long)uvm->swpages;
    data->usedSwap  = (long long)uvm->pagesize * (long long)uvm->swpginuse;
    data->freeSwap  = data->totalSwap - data->usedSwap;

    return 1;
#endif // defined(Q_OS_NETBSD) || defined(Q_OS_OPENBSD)

#ifdef Q_OS_WIN
    memstats.dwLength = sizeof(memstats);

    if (!GlobalMemoryStatusEx(&memstats))
    {
        //sg_set_error_with_errno(SG_ERROR_MEMSTATUS, "GloblaMemoryStatusEx");
        return 0;
    }

    /* the PageFile stats include Phys memory "minus an overhead".
     * Due to this unknown "overhead" there's no way to extract just page
     * file use from these numbers */
    data->totalSwap = memstats.ullTotalPageFile;
    data->freeSwap  = memstats.ullAvailPageFile;
    data->usedSwap  = data->totalSwap - data->freeSwap;

    return 1;
#endif

    return -1;
}
