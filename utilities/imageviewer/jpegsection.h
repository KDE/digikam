#ifndef JPEGSECTION_H
#define JPEGSECTION_H

class JpegSection {

    friend class ExifRestorer;

public:

    JpegSection() {
        data = 0;
        size = 0;
    }

    ~JpegSection() {
        if (data)
            delete [] data;
    }

private:

    unsigned char* data;
    unsigned int   size;
    unsigned char  type;

};


#endif
