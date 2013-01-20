#include <iostream>
#include <string.h>
#include <stdlib.h>

#include <qpdf/QPDF.hh>
#include <qpdf/QUtil.hh>

static char const* whoami = 0;

void usage()
{
    std::cerr << "Usage: " << whoami << " filename page-number" << std::endl
	      << "Prints a dump of the objects in the content streams"
              << " of the given page." << std::endl
              << "Pages are numbered from 1." << std::endl;
    exit(2);
}

class ParserCallbacks: public QPDFObjectHandle::ParserCallbacks
{
  public:
    virtual ~ParserCallbacks()
    {
    }

    virtual void handleObject(QPDFObjectHandle);
    virtual void handleEOF();
};

void
ParserCallbacks::handleObject(QPDFObjectHandle obj)
{
    if (obj.isInlineImage())
    {
        std::string val = obj.getInlineImageValue();
        std::cout << "inline image: ";
        char buf[3];
        buf[2] = '\0';
        for (size_t i = 0; i < val.length(); ++i)
        {
            sprintf(buf, "%02x", (unsigned char)(val[i]));
            std::cout << buf;
        }
        std::cout << std::endl;
    }
    else
    {
        std::cout << obj.unparse() << std::endl;
    }
}

void
ParserCallbacks::handleEOF()
{
    std::cout << "-EOF-" << std::endl;
}

int main(int argc, char* argv[])
{
    whoami = QUtil::getWhoami(argv[0]);

    // For libtool's sake....
    if (strncmp(whoami, "lt-", 3) == 0)
    {
	whoami += 3;
    }

    if (argc != 3)
    {
	usage();
    }
    char const* filename = argv[1];
    int pageno = atoi(argv[2]);

    try
    {
	QPDF pdf;
	pdf.processFile(filename);
        std::vector<QPDFObjectHandle> pages = pdf.getAllPages();
        if ((pageno < 1) || (pageno > (int)pages.size()))
        {
            usage();
        }

        QPDFObjectHandle page = pages[pageno-1];
        QPDFObjectHandle contents = page.getKey("/Contents");
        ParserCallbacks cb;
        QPDFObjectHandle::parseContentStream(contents, &cb);
    }
    catch (std::exception& e)
    {
	std::cerr << whoami << ": " << e.what() << std::endl;
	exit(2);
    }

    return 0;
}