#ifndef HAS_ETREE_DEFS_H
#define HAS_ETREE_DEFS_H

/* v_arg functions */
#define va_int(ap)     va_arg(ap, int)
#define va_charptr(ap) va_arg(ap, char *)

/* Threading can crash under Python <= 2.4.1 */
#if PY_VERSION_HEX < 0x02040200
#  ifndef WITHOUT_THREADING
#    define WITHOUT_THREADING
#  endif
#endif

/* Python 3 doesn't have PyFile_*(), PyString_*(), ... */
#if PY_VERSION_HEX >= 0x03000000
#  define PyFile_AsFile(o)                   (NULL)
#  define PyString_Check(o)                  PyBytes_Check(o)
#  define PyString_CheckExact(o)             PyBytes_CheckExact(o)
#  define PyString_FromStringAndSize(s, len) PyBytes_FromStringAndSize(s, len)
#  define PyString_FromFormat                PyBytes_FromFormat
#  define PyString_GET_SIZE(s)               PyBytes_GET_SIZE(s)
#  define PyString_AS_STRING(s)              PyBytes_AS_STRING(s)
#else
/* we currently only use three parameters - MSVC can't compile (s, ...) */
#  define PyUnicode_FromFormat(s, a, b) (NULL)
#endif

#if PY_VERSION_HEX >= 0x03000000
#  define IS_PYTHON3 1
#else
#  define IS_PYTHON3 0
#endif

#ifdef WITHOUT_THREADING
#  define PyEval_SaveThread() (NULL)
#  define PyEval_RestoreThread(state)
#  define PyGILState_Ensure() (PyGILState_UNLOCKED)
#  define PyGILState_Release(state)
#  undef  Py_UNBLOCK_THREADS
#  define Py_UNBLOCK_THREADS
#  undef  Py_BLOCK_THREADS
#  define Py_BLOCK_THREADS
#endif

#ifdef WITHOUT_THREADING
#  define ENABLE_THREADING 0
#else
#  define ENABLE_THREADING 1
#endif

/* libxml2 version specific setup */
#include "libxml/xmlversion.h"
#if LIBXML_VERSION < 20621
/* (X|HT)ML_PARSE_COMPACT were added in libxml2 2.6.21 */
#  define XML_PARSE_COMPACT  0
#  define HTML_PARSE_COMPACT 0

/* HTML_PARSE_RECOVER was added in libxml2 2.6.21 */
#  define HTML_PARSE_RECOVER XML_PARSE_RECOVER
#endif

/* added to xmlsave API in libxml2 2.6.23 */
#if LIBXML_VERSION < 20623
#  define xmlSaveToBuffer(buffer, encoding, options)
#endif

/* added to xmlsave API in libxml2 2.6.22 */
#if LIBXML_VERSION < 20622
#  define XML_SAVE_NO_EMPTY   1<<2, /* no empty tags */
#  define XML_SAVE_NO_XHTML   1<<3  /* disable XHTML1 specific rules */
#endif

/* added to xmlsave API in libxml2 2.6.21 */
#if LIBXML_VERSION < 20621
#  define XML_SAVE_NO_DECL    1<<1, /* drop the xml declaration */
#endif

/* schematron was added in libxml2 2.6.21 */
#ifdef LIBXML_SCHEMATRON_ENABLED
#  define ENABLE_SCHEMATRON 1
#  if LIBXML_VERSION < 20632
     /* schematron error reporting was added in libxml2 2.6.32 */
#    define xmlSchematronSetValidStructuredErrors(ctxt, errorfunc, data)
#    define XML_SCHEMATRON_OUT_ERROR 0
#  endif
#else
#  define ENABLE_SCHEMATRON 0
#  define XML_SCHEMATRON_OUT_QUIET 0
#  define XML_SCHEMATRON_OUT_XML 0
#  define XML_SCHEMATRON_OUT_ERROR 0
   typedef void xmlSchematron;
   typedef void xmlSchematronParserCtxt;
   typedef void xmlSchematronValidCtxt;
#  define xmlSchematronNewDocParserCtxt(doc) NULL
#  define xmlSchematronNewParserCtxt(file) NULL
#  define xmlSchematronParse(ctxt) NULL
#  define xmlSchematronFreeParserCtxt(ctxt)
#  define xmlSchematronFree(schema)
#  define xmlSchematronNewValidCtxt(schema, options) NULL
#  define xmlSchematronValidateDoc(ctxt, doc) 0
#  define xmlSchematronFreeValidCtxt(ctxt)
#  define xmlSchematronSetValidStructuredErrors(ctxt, errorfunc, data)
#endif


/* work around MSDEV 6.0 */
#if (_MSC_VER == 1200) && (WINVER < 0x0500)
long _ftol( double ); //defined by VC6 C libs
long _ftol2( double dblSource ) { return _ftol( dblSource ); }
#endif

#ifdef __GNUC__
/* Test for GCC > 2.95 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && (__GNUC_MINOR__ > 95)) 
#define unlikely_condition(x) __builtin_expect((x), 0)
#else /* __GNUC__ > 2 ... */
#define unlikely_condition(x) (x)
#endif /* __GNUC__ > 2 ... */
#else /* __GNUC__ */
#define unlikely_condition(x) (x)
#endif /* __GNUC__ */

#ifndef Py_TYPE
  #define Py_TYPE(ob)   (((PyObject*)(ob))->ob_type)
#endif

#define PY_NEW(T) \
     (((PyTypeObject*)(T))->tp_new( \
             (PyTypeObject*)(T), __pyx_empty_tuple, NULL))

#define _fqtypename(o)  ((Py_TYPE(o))->tp_name)

#define _isString(obj)   (PyString_CheckExact(obj)  || \
                          PyUnicode_CheckExact(obj) || \
                          PyObject_TypeCheck(obj, &PyBaseString_Type))

#define _isElement(c_node) \
        (((c_node)->type == XML_ELEMENT_NODE) || \
         ((c_node)->type == XML_COMMENT_NODE) || \
         ((c_node)->type == XML_ENTITY_REF_NODE) || \
         ((c_node)->type == XML_PI_NODE))

#define _isElementOrXInclude(c_node) \
        (_isElement(c_node)                     || \
         ((c_node)->type == XML_XINCLUDE_START) || \
         ((c_node)->type == XML_XINCLUDE_END))

#define _getNs(c_node) \
        (((c_node)->ns == 0) ? 0 : ((c_node)->ns->href))

/* Macro pair implementation of a depth first tree walker
 *
 * Calls the code block between the BEGIN and END macros for all elements
 * below c_tree_top (exclusively), starting at c_node (inclusively iff
 * 'inclusive' is 1).
 * 
 * To traverse the node and all of its children and siblings in Pyrex, call
 *    cdef xmlNode* some_node
 *    BEGIN_FOR_EACH_ELEMENT_FROM(some_node.parent, some_node, 1)
 *    # do something with some_node
 *    END_FOR_EACH_ELEMENT_FROM(some_node)
 *
 * To traverse only the children and siblings of a node, call
 *    cdef xmlNode* some_node
 *    BEGIN_FOR_EACH_ELEMENT_FROM(some_node.parent, some_node, 0)
 *    # do something with some_node
 *    END_FOR_EACH_ELEMENT_FROM(some_node)
 *
 * To traverse only the children, do:
 *    cdef xmlNode* some_node
 *    some_node = parent_node.children
 *    BEGIN_FOR_EACH_ELEMENT_FROM(parent_node, some_node, 1)
 *    # do something with some_node
 *    END_FOR_EACH_ELEMENT_FROM(some_node)
 *
 * NOTE: 'some_node' MUST be a plain 'xmlNode*' !
 *
 * NOTE: parent modification during the walk can divert the iterator, but
 *       should not segfault !
 */

#define _ADVANCE_TO_NEXT_ELEMENT(c_node)             \
    while ((c_node != 0) && (!_isElement(c_node)))   \
        c_node = c_node->next;

#define _TRAVERSE_TO_NEXT_ELEMENT(c_stop_node, c_node)         \
{                                                              \
    /* walk through children first */                          \
    xmlNode* ___next = c_node->children;                       \
    _ADVANCE_TO_NEXT_ELEMENT(___next)                          \
    if ((___next == 0) && (c_node != c_stop_node)) {           \
        /* try siblings */                                     \
        ___next = c_node->next;                                \
        _ADVANCE_TO_NEXT_ELEMENT(___next)                      \
        /* back off through parents */                         \
        while (___next == 0) {                                 \
            c_node = c_node->parent;                           \
            if (c_node == 0)                                   \
                break;                                         \
            if (c_node == c_stop_node)                         \
                break;                                         \
            if (!_isElement(c_node))                           \
                break;                                         \
            /* we already traversed the parents -> siblings */ \
            ___next = c_node->next;                            \
            _ADVANCE_TO_NEXT_ELEMENT(___next)                  \
        }                                                      \
    }                                                          \
    c_node = ___next;                                          \
}

#define BEGIN_FOR_EACH_ELEMENT_FROM(c_tree_top, c_node, inclusive)    \
{                                                                     \
    if (c_node != 0) {                                                \
        const xmlNode* ___tree_top = (c_tree_top);                    \
        /* make sure we start at an element */                        \
        if (!_isElement(c_node)) {                                    \
            /* we skip the node, so 'inclusive' is irrelevant */      \
            if (c_node == ___tree_top)                                \
                c_node = 0; /* nothing to traverse */                 \
            else {                                                    \
                c_node = c_node->next;                                \
                _ADVANCE_TO_NEXT_ELEMENT(c_node)                      \
            }                                                         \
        } else if (! (inclusive)) {                                   \
            /* skip the first node */                                 \
            _TRAVERSE_TO_NEXT_ELEMENT(___tree_top, c_node)            \
        }                                                             \
                                                                      \
        /* now run the user code on the elements we find */           \
        while (c_node != 0) {                                         \
            /* here goes the code to be run for each element */

#define END_FOR_EACH_ELEMENT_FROM(c_node)                             \
            _TRAVERSE_TO_NEXT_ELEMENT(___tree_top, c_node)            \
        }                                                             \
    }                                                                 \
}


#endif /* HAS_ETREE_DEFS_H */
