/**
 * section: xmlReader
 * synopsis: Parse an XML file with an xmlReader
 * purpose: Demonstrate the use of xmlReaderForFile() to parse an XML file
 *          and dump the informations about the nodes found in the process.
 *          (Note that the XMLReader functions require libxml2 version later
 *          than 2.6.)
 * usage: reader1 <filename>
 * test: reader1 test2.xml > reader1.tmp ; diff reader1.tmp reader1.res ; rm reader1.tmp
 * author: Daniel Veillard
 * copy: see Copyright for the status of this software.
 */
 
#include <stdio.h>
#include <libxml/xmlreader.h>
 
#ifdef LIBXML_READER_ENABLED
 
/**
 * processNode:
 * @reader: the xmlReader
 *
 * Dump information about the current node
 */
static void
processNode(xmlTextReaderPtr reader) {
    const xmlChar *name, *value;
    int nodetype;
 
    static char spaces[33]="                                ";

    name = xmlTextReaderConstName(reader);
    if (name == NULL)
	name = BAD_CAST "--";
 
    value = xmlTextReaderConstValue(reader);
 
    nodetype = xmlTextReaderNodeType(reader);
    if (nodetype!=14 && nodetype!=15)	// Skip all text bodies and "/" entities.
    {
#ifdef DETAILS
    printf("%s%d %d %s %d %d", 
    	    (spaces + (32-2*xmlTextReaderDepth(reader))),
	    xmlTextReaderDepth(reader),
	    nodetype,
	    name,
	    xmlTextReaderIsEmptyElement(reader),
	    xmlTextReaderHasValue(reader));
#else
    printf("%s%s", (spaces + (32-2*xmlTextReaderDepth(reader))), name);
#endif

    if (value == NULL)
	printf("\n");
    else {
        if (xmlStrlen(value) > 40)
            printf(" %.40s...\n", value);
        else
	    printf(" %s\n", value);
    }
    }

    if (nodetype==1)	// Element - possibly has attributes
    {
	while (xmlTextReaderMoveToNextAttribute(reader))
	    printf("%s    Attrib: %s=\"%s\"\n", 
		(spaces + (32-2*xmlTextReaderDepth(reader))),
		xmlTextReaderConstName(reader), 
		xmlTextReaderConstValue(reader));
    }

}
 
/**
 * streamFile:
 * @filename: the file name to parse
 *
 * Parse and print information about an XML file.
 */
static void
streamFile(const char *filename) {
    xmlTextReaderPtr reader;
    int ret;
 
#define VALIDATEDTD
#ifdef VALIDATEDTD
    /*
     * Pass some special parsing options to activate DTD attribute defaulting,
     * entities substitution and DTD validation
     */
    reader = xmlReaderForFile(filename, NULL,
                 XML_PARSE_DTDATTR |  /* default DTD attributes */
		 XML_PARSE_NOENT |    /* substitute entities */
		 XML_PARSE_DTDVALID); /* validate with the DTD */
#else
    reader = xmlReaderForFile(filename, NULL, 0);
#endif
    if (reader != NULL) {
        ret = xmlTextReaderRead(reader);
        while (ret == 1) {
            processNode(reader);
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeTextReader(reader);
        if (ret != 0) {
            fprintf(stderr, "%s : failed to parse\n", filename);
        }
    } else {
        fprintf(stderr, "Unable to open %s\n", filename);
    }
}
 
int main(int argc, char **argv) {
    if (argc != 2)
        return(1);
 
    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION
 
    streamFile(argv[1]);
 
    /*
     * Cleanup function for the XML library.
     */
    xmlCleanupParser();
    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();
    return(0);
}
 
#else
int main(void) {
    fprintf(stderr, "XInclude support not compiled in\n");
    exit(1);
}
#endif

