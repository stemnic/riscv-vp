/*  wrapper.h 1.125 2018/12/21
  
    Copyright 1998-2018 by Concept Engineering GmbH.
    All rights reserved.
    ===========================================================================
    This source code belongs to Concept Engineering.  It is considered
    trade secret, and is not to be divulged or used or copied by parties
    who have not received written authorization from the owner, Concept
    Engineering.
    ===========================================================================
    Title:
        NlviewQT - Wrapper header file
    ===========================================================================
*/

#ifndef WRAPPER_H_
#define WRAPPER_H_

/*! \mainpage NlviewQT documentation
 *
 * [core API]: ../nlviewCore.html "Nlview Core API"
 *
 * <b>About this manual</b>
 *
 * <IMG SRC="../img/nlviewQTOv.gif" ALIGN="right" HSPACE="10" ALT="API Overview">
 *
 * This reference manual for NlviewQT covers the Qt related issues
 * of the product. The GUI independent description of the Nlview widget
 * family can be found in the [Nlview core manual][core API].
 *
 * There is also a small <a href="../README_QT.txt">README</a>
 * file containing step by step
 * installation and compilation instructions - as well as setting up
 * a valid
 * <a href="../flexlm_install.html">license</a>
 * for the widget.
 *
 * NlviewQT consists of the Qt specific wrapper class called #NlvQWidget
 * and the (GUI independent) Nlview core library.  While the core library
 * is delivered in machine-dependent object format, the Qt specific wrapper
 * is delivered in source code.  This source code
 * is compatible with all Qt versions from Qt 3.x to Qt 5.x.
 *
 * By default, the <a href="../nlviewCore.html#PROP_tclcompat">tclcompat</a>
 * is false, but can be set to true at compile time with -DNLV_TCLCOMPAT.
 *
 *
 * <BR clear=all>
 */

/*!
 * \file wrapper.h
 * \brief
 *   This is the NlviewQT Wrapper header file.
 *
 * This is the interface of a Qt class that acts as a
 * wrapper for the generic (GUI independent) Nlview core library.
 * It hides the internals of Nlview's core library interface from the
 * user and maps its functionality to Qt compliant functions.
 * Your Qt application code needs to do:
 *
   \code{.cpp}
   #include "wrapper.h"
   \endcode
 *
 * The documentation for class NlvQWidget contains more details.
 *
 * \author Jochen Roemmler
 * \date 2018/12/21
 * \version 1.125
 */

#include <qglobal.h>
#if QT_VERSION >= 0x040000	// Qt4+
#include <QColor>
#include <QCursor>
#include <QFont>
#include <QList>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include <QWidget>
#include <iostream>
#else				// Qt 2.x - 3.x
#include <qcolor.h>
#include <qcursor.h>
#include <qfont.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qwidget.h>
#if QT_VERSION >= 300		// Qt 3.x only
#include <qptrlist.h>
#else				// Qt 2.x only
#include <qlist.h>
#endif
#endif // QT_VERSION


#ifndef SKIP_DOCUMENTATION
// ===========================================
// forward declarations
// ===========================================

class QPainter;
class QPixmap;
class QMouseEvent;
class QwheelEvent;
class QPaintEvent;
class QResizeEvent;
class QScrollBar;
struct WidgetImpl;
struct NlvCore;
struct NlvRend;
class NlvQWidget;
class ImageMap;
struct WBox;

#define NLV struct NlvWidget
#ifdef Q_NO_BOOL_TYPE
#define true  1
#define false 0
#endif

// ===========================================
// Windows: declare CLASS_EXPORT macro for DLL
// ===========================================
#if (defined(_WIN32) || defined(__CYGWIN32__)) && !defined(WIN)
#define WIN
#endif

#ifdef WIN
#  ifdef NLVIEW_DLLEXPORT
#    define CLASS_EXPORT __declspec( dllexport )
#  else
#    ifdef NLVIEW_DLLIMPORT
#      define CLASS_EXPORT __declspec( dllimport )
#    else
#      define CLASS_EXPORT /* neither EXPORT nor IMPORT: static build */
#    endif
#  endif
#else
#  define CLASS_EXPORT /* Not used for UNIX */
#endif


// ******************************************************************
// Internal use only.
//
// The Nlview C core callback functions for the function vector in
// struct NlvWidget must have C linkage. Most of those functions
// must call private member functions of class NlvQWidget.
// To allow this, we must declare them 'friend'.
// Before we do this, we pre-declare them 'extern "C"', because
// otherwise they will implicitly become 'extern' (C++ linkage) instead of
// 'extern "C"' (C linkage).
// ******************************************************************
#ifdef DECLARE_KERNEL_FUNC
//
// The DECLARE_KERNEL_FUNC define is only enabled when this file is included
// from wrapper.cpp; we must walk around a problem when compiling the 
// meta-object code, which includes this header file, too.
// Certain compilers (gcc 3.x) complain about missing implementations for the
// functions declared below:
//
extern "C" {
    static void vDefineScrollregion(  NLV*,WBox);
    static void vUndefineScrollregion(NLV*);
    static void vDamageAll(           NLV*);
    static void vDamage(              NLV*,WBox);
    static void vResetAll(            NLV*);
    static void vCenterAt(            NLV*,int,int);
    static void vMoveCursorTo(        NLV*,int,int);
    static void vOwnSelection(        NLV*);
    static void vPageNotify(          NLV*,unsigned,int,int);
    static void vSelectionNotify(     NLV*,unsigned);
    static void vHierarchyGodown(     NLV*,const char*);
    static void vHierarchyGoup(       NLV*);
    static void vBindcallback(        NLV*,const char*,int,int,int,int,
							int,const char*);
    static void vMessageOutput(       NLV*,const char*,const char*);
    static void vProgressNotify(      NLV*,int,float,const char*);
    static void vPropertyNotify(      NLV*,const char*,const char*);
    static void vWaitInMainloop(      NLV*,int);
    static void vGetElidedText(       NLV*,char*,const char*,int,int,
					    int,const char*,int);
    static void vBusyCursor(          NLV*);
    static void vStdCursor(           NLV*);
    static void vStartPan(            NLV*,int,int);
    static void vPanTo(               NLV*,int,int);
    static void vStopPan(             NLV*,int,int);
    static void vInvalidate(          NLV*,WBox);
}
#endif // DECLARE_KERNEL_FUNC

/*!
 * \internal
 * This is an internal helper class that implements a dynamically growing
 * string-pool and is used internally by classes NlviewArgs and NlviewList.
 * It guarantees that pointers to inserted strings are never moved during
 * the object's life-time allowing callers to store them permanently.
 */
class NlviewStrSpace {
 private:
	NlviewStrSpace(const char* s = NULL);
	virtual ~NlviewStrSpace();
	const char* append(const char* s);
	void clear();
	void insertBlock(unsigned);
	NlviewStrSpace(const NlviewStrSpace&);		  /*!< unimplemented */
	NlviewStrSpace& operator= (const NlviewStrSpace&);/*!< unimplemented */

	enum {DEFAULT_BLOCK_SIZE = 2048};

	struct MemBlock* root;

	friend class NlviewArgs;
	friend class NlviewList;
};
#endif // SKIP_DOCUMENTATION

/*!
 * \class NlviewArgs
 * \brief
 *   Helper class to incrementally build a list of arguments
 *   (to call Nlview commands).
 *
 * This helper class allows convenient and incremental building of an
 * argument list in C++ style using overloaded #operator<< .
 * Objects of this class can only be used by
 * \li NlvQWidget::command(bool*) or
 * \li NlvQWidget::command(bool*,NlviewArgs&) or
 * \li #operator<<(const NlviewArgs&) to append it as a sub-list
 *     argument to a different instantiation.
 *
 * \par Example using NlvQWidget::command(bool*):
   \code{.cpp}
   nlview->args << "zoom" << 2.5;
   nlview->command(&ok);
   \endcode
 *
 * \par Example using NlvQWidget::command(bool*,NlviewArgs&):
   \code{.cpp}
   NlviewArgs myargs;
   myargs << "load" << "port" << "my port" << "input";
   nlview->command(&ok, myargs);
   \endcode
 *
 * \par Example using it as a sub-list in another NlviewArgs object:
   \code{.cpp}
   NlviewArgs portID;
   portID << "port" << "my port";
   nlview->args << "highlight" << "-itemized" << portID;
   nlview->command(&ok);
   \endcode
 *
 * After calling
 * NlvQWidget::command(bool*) or
 * NlvQWidget::command(bool*,NlviewArgs&)
 * the NlviewArg object will be cleared, ready to receive new arguments.
 * It's a good idea to use the same NlviewArg object for multiple calls,
 * because internal memory can be re-used.
 */
class CLASS_EXPORT NlviewArgs
{
 public:
	/*!
	 * \brief
	 *   Constructs an empty NlviewArgs object.
	 *
	 * The object is ready to be filled with arguments
	 * using one of the overloads of #operator<<.
	 */
	NlviewArgs();

	/*!
	 * \brief
	 *   Copy constructor to initialize a new object from another
	 *   existing object.
	 *
	 * The string memory occupied by \c args will be duplicated.
	 */
	NlviewArgs(const NlviewArgs& args);

	/*!
	 * \brief
	 *   Destructor - free internal memory.
	 */
	virtual ~NlviewArgs();

	/*!
	 * \brief
	 *   Assignment operator to re-initialize this object with a
	 *   argument list from another existing object.
	 *
	 * The string memory occupied by \c args will be duplicated.
	 * Do not confuse with #operator<<(const NlviewArgs&) which
	 * will append a sub-list to this object's argument list.
	 */
	NlviewArgs& operator= (const NlviewArgs& args);

	/*!
	 * \brief
	 *   Appends the given null-terminated C string to the
	 *   list of arguments.
	 *
	 * The string's memory will be duplicated.
	 */
	NlviewArgs& operator<< (const char* arg);

	/*!
	 * \brief
	 *   Appends the given null-terminated C++ string to the
	 *   list of arguments.
	 *
	 * The string's memory will be duplicated.
	 */
	NlviewArgs& operator<< (const std::string& arg);

	/*!
	 * \brief
	 *   Appends the given QString to the list of arguments.
	 *
	 * The string's memory will be duplicated (deep copy).
	 */
	NlviewArgs& operator<< (const QString& arg);

	/*!
	 * \brief
	 *   Appends the arguments of given object as a (single) sub-list
	 *   to the list of arguments.
	 *
	 * This operator is helpful to build Nlview commands which
	 * are taking an object id or other (sub-)lists as argument.
	 * The memory of all arguments from source will be copied.
	 *
	 * This is an overloaded operator, provided for convenience.
	 * It behaves essentially like #operator<<(const char*).
	 */
	NlviewArgs& operator<< (const NlviewArgs& arg);

	/*!
	 * \brief
	 *   Appends the given int argument to the list of arguments.
	 *
	 * This operator is helpful to build Nlview commands which
	 * are expecting an integer value as argument -
	 * avoiding the need to convert it into a string first.
	 *
	 * This is an overloaded operator, provided for convenience.
	 * It behaves essentially like #operator<<(const char*).
	 */
	NlviewArgs& operator<< (int arg);

	/*!
	 * \brief
	 *   Appends the given double argument to the list of arguments.
	 *
	 * This operator is helpful to build Nlview commands which
	 * are expecting a floating point number as argument -
	 * avoiding the need to convert it into a string first.
	 *
	 * This is an overloaded operator, provided for convenience.
	 * It behaves essentially like #operator<<(int).
	 */
	NlviewArgs& operator<< (double arg);

	/*!
	 * \brief
	 *   Read-only access a single argument by index.
	 *
	 * This operator returns a pointer to a
	 * null-terminated C-string at the given position in the argument list,
	 * which has been inserted before.
	 * The index must be between 0 and #length()-1. The pointer is
	 * valid until the end of the object's life-time or the object is
	 * used to execute a Nlview command or #reset is called.
	 *
	 * \param index The 0-based index of the argument to access.
	 * \returns \c NULL if index is out of range, or the addressed argument.
	 */
	const char* operator[] (int index) const;

	/*!
	 * \brief
	 *   Returns the number of arguments inserted so far.
	 */
	int length() const;

	/*!
	 * \brief
	 *   Clears all arguments and starts with an empty argument list.
	 */
	void reset();


 private:
	/*!
	 * \brief
	 *   Read-only access to internal data representation
	 *   (whole array of arguments).
	 *
	 * Returns a list of null-terminated C strings of all arguments
	 * inserted so far. The \b life-time of the string
	 * list is limited to the next call of an #operator<<
	 * and to the life-time of the object, of course.
	 *
	 * \returns \c NULL or a list of NULL-terminated C strings.
	 */
	const char** operator()() const;

	friend class NlvQWidget;
	friend class NlviewList;

 private:
	NlviewStrSpace buf;	/*!< used as a dynamically growing char[] */
    	const char**   argv;	/*!< dynamically growing string-list */
    	int            argc;	/*!< number of elements in argv */
    	int            size;	/*!< total array size of argv */
};

/*!
 * \class NlviewList
 * \brief
 *   Helper class to split Tcl-like \ref lists "list" into its elements
 *   (e.g. to parse results from Nlview commands).
 *
 * This helper class basically acts as a wrapper around
 * NlvQWidget::splitList and NlvQWidget::freeList functions.
 * It allows convenient, safe and C++-style access to individual arguments
 * of a Tcl-like \ref lists "list", e.g. as returned by NlvQWidget::command.
 * Moreover, the list passed as argument to the constructor is copied
 * into private memory, which can be useful to buffer the (volatile)
 * result string returned by NlvQWidget::command.
 *
 * Used as an object created on the stack, it also helps to free the
 * memory allocated by
 * NlvQWidget::splitList by running
 * NlvQWidget::freeList in the destructor.
 *
 * You must call #split at least once, before you can access the list elements;
 * except for #isEmpty - which works without a prior call to #split.
 * Repeated calls to #split are always fast.
 *
 * A NlviewList object cannot change its data - only the
 * \ref NlviewList::NlviewList "constructor", assignment or + operators
 * can set the data. The class declares a cast-operator to type
 * \c const \c char* offering transparent use as a null-terminated C string.
 *
 * \par Example 1: split method and operator[]
   \code{.cpp}
   // process selected Nlview objects
   NlviewList selection_list = nlview->command(&ok, "selection");
   if (!ok || selection_list.isEmpty()) return;
   int len = selection_list.split();
   for (int i=0; i<len; i++)
      // do something with selection[i]
   \endcode
 *
 * \par Example 2: cast operator const char*
   \code{.cpp}
   NlviewList oid_list = nlview->command(&ok, "selection");
   // result is now buffered in oid_list
   ...
   nlview->command(&ok, "highlight", oid_list);
   \endcode
 *
 * \sa NlviewList operator+(const NlviewList&, const NlviewList&)
 */
class CLASS_EXPORT NlviewList
{
 public:
	/*!
	 * \brief
	 *   Constructs an new NlviewList object,
	 *   initialized with given merged list.
	 *
	 * The given list must be a null-terminated C string
	 * consisting of a merged Tcl-like \ref lists "list", e.g.
	 * as returned by NlvQWidget::command.
	 * The passed string will be copied into private memory;
	 * so this class can also be used to buffer any result
	 * string returned from NlvQWidget::command function.
	 * The passed string may be \c NULL (is equivalent to "").
	 *
	 * Call #length method to get hold of the number of elements in the
	 * list, and use #operator[] to read individual list elements.
	 */
	NlviewList(const char* list = NULL);

	/*!
	 * \brief
	 *   Copy constructor to initialize a new object from another
	 *   existing object.
	 *
	 * The string memory occupied by \c other will be duplicated.
	 */
	NlviewList(const NlviewList& other);

	/*!
	 * \brief
	 *   Constructs an new NlviewList object,
	 *   initialized with given argument list.
	 *
	 * While it might seem unnecessary to initialize this class
	 * with an argument list (which is split into arguments by definition)
	 * it is quite handy to misuse this class to convert the given
	 * argument list into a single (merged) string accessible
	 * with #operator const char*().
	 * The string memory from args object will be copied.
	 */
	NlviewList(const NlviewArgs& args);

	/*!
	 * \brief
	 *   Assignment operator to re-initialize this object from another
	 *   existing object.
	 *
	 * This operator will invalidate all references returned by
	 * #operator const char*() and #operator[].
	 * The string memory occupied by \c other will be copied.
	 */
	NlviewList& operator= (const NlviewList& other);

	/*!
	 * \brief
	 *   Assignment operator to re-initialize this object from another
	 *   existing argument list object.
	 *
	 * For reasons on why one would want to do that see the 
	 * \ref NlviewList(const NlviewArgs& args) "constructor".
	 *
	 * This operator will invalidate all references returned by
	 * #operator const char*() and #operator[].
	 * The string memory occupied by \c args will be copied.
	 */
	NlviewList& operator= (const NlviewArgs& args);

	/*!
	 * \brief
	 *   Operator to append another list to this list.
	 *
	 * This operator appends the string stored in the \c other object
	 * to \c this object.
	 *
	 * This operator will invalidate all references returned by
	 * #operator const char*() and #operator[].
	 */
	NlviewList& operator+= (const NlviewList& other);

	/*!
	 * \brief
	 *   Destructor - free internal memory.
	 *
	 * Any pointer returned by #operator[] (reference into the list)
	 * or cast #operator const char*() (reference to unsplit list)
	 * will become invalid afterwards.
	 */
	virtual ~NlviewList();

	/*!
	 * \brief
	 *   Quickly tell whether or not the list contains no elements at all.
	 *
	 * This method is a fast alternative to checking #length() == 0.
	 * This function is provided for convenience and performance.
	 *
	 * \returns \c true if the list does not contain any elements at all.
	 */
	bool isEmpty() const;

	/*!
	 * \brief
	 *   Splits the list into its elements and returns the
	 *   number of elements (or -1 on error).
	 *
	 * This method splits the \ref lists "Tcl-list"
	 * (stored as a merged string) into its elements and returns
	 * the number of elements (or -1 on error).
	 *
	 * Multiple calls to #split have no (side-)effect and are always fast.
	 *
	 * \returns -1 on error, else the number of list elements
	 */
	int split() const;

	/*!
	 * \brief
	 *   Read-only access a single list element by index.
	 *
	 * This operator returns the indexed element or \c NULL (if index is
	 * out of range).
	 * The \b life-time of the string
	 * memory of the returned list element is limited.
	 *
	 * The index is expected to be between 0 and #length()-1.
	 * This operator implicitly calls #split.
	 *
	 * \param index The 0-based index of the list element to access.
	 * \returns \c NULL if index is out of range, or the addressed
	 *            list element.
	 */
	const char* operator[] (int index) const;

	/*!
	 * \brief
	 *   cast operator to \c const \c char*
	 *
	 * This cast operator is provided for convenience.
	 * It returns a pointer to the \b unsplit
	 * \c null-terminated C string; this is the string
	 * which was passed to the
	 * \ref NlviewList::NlviewList "constructor" or assignment operators
	 * (or modified by #operator+=).
	 * It returns \c NULL if the input string was \c NULL already.
	 * The \b life-time of the resulting string is limited.
	 *
	 * \returns string representation of the unsplit list
	 * \sa #getText
	 */
	operator const char*() const { return list; }

	/*!
	 * \brief
	 *    returns the same pointer as the `const char*` cast operator
	 *
	 * This function is provided for convenience. It
	 * returns the same pointer as the #operator const char*().
	 */
	const char* getText() const { return list; }

	/*!
	 * \brief
	 *   Comparison operator.
	 *
	 * The comparison operator \c == is provided for convenience.
	 * It compares the given NlviewList object with this object.
	 *
	 * \param other The object that we want to compare with
	 * \returns \c true if both list objects are considered equal
	 *          (case sensitive), \c false otherwise.
	 */
	bool operator==(const NlviewList& other) const;

	/*!
	 * \brief
	 *   Returns the number of list elements found.
	 *
	 * This method implicitly calls #split.
	 */
	int length() const;

	/*!
	 *   This enum is used for the #objType and
	 *   NlvQWidget::getCustomElidedText function.
	 */
	enum OType {
		O_Error      =  0,	/*!< indicates an error */
		O_Port       =  1,	/*!< represents a port object_id */
		O_PortBus    =  2,	/*!< represents a portBus object_id */
		O_Pin        =  3,	/*!< represents a pin object_id */
		O_PinBus     =  4,	/*!< represents a pinBus object_id */
		O_Segm       =  5,	/*!< represents a segm object_id */
		O_Segm1io    =  6,	/*!< represents a segm1io object_id */
		O_Segm0io    =  7,	/*!< represents a segm0io object_id */
		O_Inst       =  8,	/*!< represents an inst object_id */
		O_Net        =  9,	/*!< represents a net object_id */
		O_NetVector  = 10,	/*!< represents a netVector object_id */
		O_NetBundle  = 11,	/*!< represents a netBundle object_id */
		O_Symbol     = 12,	/*!< represents a symbol object_id */
		O_Attr       = 13,	/*!< represents an attr object_id */
		O_Text       = 14,	/*!< represents a text object_id */
		O_Image      = 15,	/*!< represents a image object_id */
		O_Button     = 16,	/*!< represents a button object_id */
		O_HierPin    = 17,	/*!< represents a hierPin object_id */
		O_HierPinBus = 18,	/*!< represents a hierPinBus object_id*/
		O_PageSubNet = 19,	/*!< represents a net object_id */
		O_CGraphic   = 20,	/*!< represents a cgraphic object_id */
		O_NetWire    = 21,	/*!< represents a netwire object_id */
		O_Last       = 22	/*!< reserved for internal use */
	};

	/*!
	 * \brief
	 *   Returns the object_id type from the first list element `argv[0]`.
	 *
	 * [object_id]: ../nlviewCore.html#IDENTIFICATION "Nlview object id"
	 *
	 * This method implicitly calls the #split method before it
	 * looks up the first list element (argv[0]) and returns
	 * an enumerator from enum #OType.
	 *
	 * A NlviewList can store any list, but one "special list"
	 * is the [object_id]
	 * whose list elements have the following meaning:
	 * \c argv[0] = <object-type>,
	 * \c argv[1] = <object-name>,
	 * \c argv[2] = ...
	 * If so, then #objType can be called to get an enumerator
	 * from the object type at argv[0].
	 * #objType is a convenience function and is much quicker
	 * than string comparison, because it internally uses perfect
	 * hash lookup.
         * If \c NlviewList does \b not store an [object_id],
	 * then calling #objType makes no sense (result will be \c O_Error). 
	 *
	 * \returns \c O_Error or type of [object_id] stored in the list.
	 */
	enum OType objType() const;

	/*!
	 * \brief
	 *   Convert a given string decimal representation of a number
	 *   to an integer value.
	 *
	 * If endp is given, then it points to the end of the number
	 * representation in the given string - if the given string only
	 * stores the number, then endp should point to the end-of-string.
	 */
	static int toInt(const char* arg, const char** endp = 0);

	/*!
	 * \brief
	 *   Convert a given string representation of a floating point number
	 *   in C locale to a double value.
	 *
	 * The format of the number is not localized;
	 * the default C locale is used irrespective of the user's locale.
	 * If endp is given, then it points to the end of the number
	 * representation in the given string - if the given string only
	 * stores the number, then endp should point to the end-of-string.
	 */
	static double toDouble(const char* arg, const char** endp = 0);

 private:
	NlviewStrSpace buf;	/*!< buffer to store string from c'tor */
	const char*    list;	/*!< pointer to stored string from c'tor */
	int            argc;	/*!< valid only after #split was called */
	const char**   argv;	/*!< valid only after #split was called */
};

/*!
 * \brief
 *   returns a list which is the result of concatenating l1 and l2.
 *
 * This operator will create a new \c NlviewList object holding a list
 * consisting of elements from both l1 and l2.
 */
inline const NlviewList operator+(const NlviewList& l1, const NlviewList& l2)
    { NlviewList l(l1); l += l2; return l; }



/*!
 * \class NlviewMinimap
 * \brief
 *   A service class that implements an interactive mini map
 *   to view, pan and zoom a coupled Nlview widget.
 *
 * This service class is an add-on to the Nlview main class NlvQWidget.
 * It implements an interactive navigation help to view, pan and zoom the
 * schematic in a separate widget.
 *
 * The contents of this widget will be a scaled version (map) of the linked
 * Nlview schematic that fits exactly into the current NlviewMinimap window.
 * The viewport of the linked Nlview widget will be visualized as a control
 * on top of this map - similar to a 2D scrollbar slider;
 * Additionally the viewport will contain size grips to easily adjust the
 * scale of the linked Nlview accordingly by dragging the sizers with
 * the left mouse button.
 *
 * A click anywhere in the map with the left mouse button will center
 * the linked Nlview on the corresponding spot, while dragging the left
 * mouse button over the map immediately scrolls the linked Nlview
 * accordingly.
 *
 * The minimap can be customized by properties and by reimplementation.
 *
 * \anchor example
 * \par Example (Qt4.3+):
   \code{.cpp}
    ...
    QDockWidget* minimapdock = new QDockWidget("MiniMap", this);
    minimapdock->setMinimumSize(QSize(100,100));
  
    NlviewMinimap* minimap = new NlviewMinimap(minimapdock);
    nlview->linkMinimap(minimap);
    minimapdock->setWidget(minimap);
    addDockWidget(Qt::BottomDockWidgetArea, minimapdock);
    ...
   \endcode
 */
class CLASS_EXPORT NlviewMinimap : public QWidget
{
	Q_OBJECT

	Q_PROPERTY(bool   hiselvis READ getHiSelVis   WRITE setHiSelVis)
	Q_PROPERTY(QColor vpcolor  READ getVportColor WRITE setVportColor)

 public:
	/*!
	 * \brief
	 *   Constructs an new NlviewMinimap object.
	 *
	 * You must pass the created object to NlvQWidget::linkMinimap
	 * in order to link it up with an existing Nlview window.
	 *
	 * \param[in] parent Pointer to a Qt parent widget or NULL.
	 * \param[in] f      Qt window flags.
	 * \returns New minimap instance.
	 * \sa \ref example
	 */
	NlviewMinimap(QWidget* parent = NULL,
#if QT_VERSION >= 0x050700				// Qt5.7+
		      Qt::WindowFlags f = Qt::WindowFlags()
#elif QT_VERSION >= 0x050000				// Qt5..5.7
		      Qt::WindowFlags f = Qt::Widget
#else							// Qt2..4
		      Qt::WFlags f = 0
#endif
);

	/*!
	 * \brief
	 *   Destructor for a NlviewMinimap object.
	 *
	 * This destructor frees all memory which has been allocated
	 * by this object.
	 * If the minimap is still linked to an existing Nlview window
	 * it will automatically be unlinked from it in this destructor.
	 * The QObject destructor will take care of this automatically.
	 */
	virtual ~NlviewMinimap();

	/*!
	 * \brief
	 *   Returns linked Nlview object
	 *
	 * \returns \c NULL if not linked to a Nlview window,
	 *          otherwise returns the Nlview object currently linked to.
	 */
	NlvQWidget* getLinkedNlview() const { return nlview; };

	/*!
	 * \brief
	 *   Returns visibility of highlighted/selected Nlview objects.
	 *
	 * \returns \c true if highlighted/selected Nlview objects should
	 *          be visible in this minimap.
	 * \sa #setHiSelVis
	 */
	bool     getHiSelVis() const { return hiselvis; };

	/*!
	 * \brief
	 *   Sets visibility of highlighted/selected Nlview objects.
	 *
	 * \param[in] visibility \c true if highlighted/selected Nlview
	 *                       objects should be visible in this minimap.
	 * \sa #getHiSelVis
	 */
	void     setHiSelVis(bool visibility) { hiselvis = visibility; };

	/*!
	 * \brief
	 *   Returns the color to be used to visualize the current viewport.
	 *
	 * The default color is the linked Nlview's rubberbandcolor.
	 *
	 * \returns viewport color.
	 * \sa #setVportColor
	 */
	QColor   getVportColor() const { return vpcolor; };

	/*!
	 * \brief
	 *   Sets the color to be used to visualize the current viewport.
	 *
	 * The default color is the linked Nlview's rubberbandcolor.
	 *
	 * \param[in] color the new viewport color to use
	 * \sa #getVportColor
	 */
	void     setVportColor(const QColor& color);

 public slots:
	/*!
	 * \brief
	 *   This method is responsible to cancel an ongoing interactive
	 *   manipulation.
	 *
	 * It will restore the schematic viewport to its
	 * original scale and scroll position as good as possible.
	 * This class reimplements the \c keyPressEvent event handler to call
	 * #cancel in response to a \c ESC key press.
	 *
	 * \sa #keyPressEvent
	 */
	virtual void cancel();

	/*!
	 * \brief
	 *   Manually mark the map as invalid and schedule a paint event
	 *   to perform the actual update.
	 *
	 * Usually you don't need to call this function, because
	 * the linked Nlview window will take care of updating
	 * the map automatically; but under some circumstances it may be
	 * required to trigger a manual update from outside Nlview.
	 */
	virtual void forceUpdate();

 private slots:
#ifndef SKIP_DOCUMENTATION
	/*!
	 * \internal
	 * \brief
	 *   This method is a slot that will be connected to
	 *   signal NlvQWidget::viewportChanged.
	 *
	 * It is responsible to update the size and position of the
	 * interactive viewport rectangle.
	 * Not to be called from user code.
	 */
	virtual void viewportChanged(const QRect&,const QPoint&);

	/*!
	 * \internal
	 * \brief
	 *   This method is a slot that will be connected to
	 *   signal NlvQWidget::mapChanged.
	 *
	 * It is responsible to update the contents of the interactive minimap
	 * from its linked Nlview.
	 * Not to be called from user code.
	 */
	virtual void mapChanged(const QRect& scrollr);
#endif

 protected:
	/*!
	 * \brief
	 *   Provides a hook to draw a custom background.
	 *
	 * It will be called immediately before the minimap is painted.
	 * The default implementation just returns \c false,
	 * which means that the minimap should be painted opaquely
	 * (using the same background as the linked Nlview window).
	 * Reimplement that method to draw whatever you like in the
	 * given rectangle using the given QPainter and
	 * return \c true, which means that the minimap should be
	 * painted transparently on top of your custom background.
	 *
	 * \param[in] p    QPainter object to paint the background with
	 * \param[in] rect The background area to be painted
	 * \returns \c true if a custom background was painted,
	 *          \c false otherwise (draw map fully opaque on top)
	 */
	virtual bool drawBackground(QPainter* p, const QRect& rect);

	/*!
	 * \brief
	 *   Reimplemented mouse button/wheel handlers for mouse interaction.
	 */
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);
	virtual void wheelEvent(QWheelEvent*);
	/*!
	 * \brief
	 *   Reimplemented key event handler for cancel() functionality.
	 *
	 * This implementation will cancel an ongoing minimap action if
	 * the ESC key is pressed.
	 *
	 * In order to enable this event handler, the constructor is
	 * setting keyboard focus policy with a call
	 * to \c QWidget::setFocusPolicy.
	 */
	virtual void keyPressEvent(QKeyEvent*);
	/*!
	 * \brief
	 *   Reimplemented to draw the minimap and viewport controls on the
	 *   screen.
	 */
	virtual void paintEvent(QPaintEvent*);
	/*!
	 * \brief
	 *   Reimplemented to return a reasonable preferable initial size.
	 */
	virtual QSize sizeHint() const { return QSize(100,100); };

 private:
	/*!
	 * \internal
	 * The different states of our "mouse" FSM.
	 */
	enum action { A_NONE, A_MOVING,
		      A_SCALING_UL, A_SCALING_UC, A_SCALING_UR,
		      A_SCALING_CL,               A_SCALING_CR,
		      A_SCALING_LL, A_SCALING_LC, A_SCALING_LR };
	/*!
	 * \internal
	 * Links the minimap to a given Nlview object.
	 * This function is called internally from NlvQWidget::linkMinimap
	 * code. Not to be called by user code.
	 */
	void linkNlview(NlvQWidget*);

	/*!
	 * \internal
	 * Updates mouse cursor shape depending on its location.
	 */
	enum action update_cursor_shape(const QPoint&, bool);

	/*!
	 * \internal
	 * Called from paintEvent to update the contents of the minimap pixmap.
	 */
	void update_map();

	/*!
	 * \internal
	 * Convert from/to "map" space (WINDOW) to/from WIDGET space.
	 */
	QPoint map2widget(const QPoint&) const;
	QPoint widget2map(const QPoint&) const;
	QRect  widget2map(const QRect&) const;

	/*!
	 * \internal
	 * Helper functions.
	 */
	bool viewportInvalid() const;
	void scaleGrip(const QRect&);

 private:
	// properties
	bool        hiselvis;   /*!< print highlighted / selected Nlview OIDs */
	QColor      vpcolor;	/*!< color to visualize current viewport */

	// private variables
	bool        vpcolor_set;/*!< true if explicit vpcolor is set */
	NlvQWidget* nlview;	/*!< pointer to linked Nlview */
	struct NlvRend* printer;/*!< used to print into the "map" */
	QPixmap*    map;	/*!< (captured) map itself */
	bool        mapdirty;	/*!< do we need to call update_map? */
	int         gripsz;	/*!< (auto-scaled) size of size-grip */

        // map_*: Nlview transformation while printing into the minimap pixmap
	int         map_xoff;	/*!< dx of Nlview's origin inside pixmap     */
	int         map_yoff;	/*!< dy of Nlview's origin inside pixmap     */
	double      map_zoom;	/*!< zoom factor of minimap schematic        */
	QRect       map_scrollregionM;/*!< MAP coords of widget's scrollreg. */
	QRect       viewport;	/*!< WIDGET coords of cur. visible rect      */
	QRect       scrollr;	/*!< WIDGET coords of complete schematic     */
	QPoint      origin;	/*!< WIDGET x/y offset, NOT necessarily
				 *   coincident w/viewport left/top corner   */
	double      w2m_scale;	/*!< rel. scale btw. Nlview and its minimap  */

	// saved values for both A_MOVING and A_SCALING_*
	enum action what;	/*!< current FSM state                       */
	QPoint      clickptM;	/*!< MAP mouse-click point                   */
	QPoint      old_origin;	/*!< WIDGET origin (scrollpos) before click  */

	// saved values for A_SCALING_* only:
	QRect       old_viewportM;/*!< MAP coords of viewport before scale   */
	double      old_zoom;	/*!< old Nlview zoom factor before scale     */
	double      max_zoom;	/*!< store property maxzoom                  */
	double      ratioM;	/*!< MAP: viewport aspect ratio (fix)        */

	friend class NlvQWidget;
};


/*!
 * \class NlvQWidget
 *
 * \brief
 *   This is the main class for NlviewQT.  It derives from the
 *   Qt base class QWidget and implements the Nlview
 *   core API.
 *
 * [core API]: ../nlviewCore.html               "Nlview Core API"
 * [lists]:    ../nlviewCore.html#OVERVIEW_LIST "how to deal with lists"
 * [load]:     ../nlviewCore.html#LOADING       "Core API: the load command"
 * [print]:    ../nlviewCore.html#PRINTING      "Core API: the print command"
 *
 * Its main purpose is to map the generic Nlview "core" API into the
 * <a href="http://qt.digia.com">Qt</a> world:
 *
 \verbatim
   +-----------------+     |     +---------------------------+ 
   | Nlview core:    |     |     | FILE: wrapper.cpp         |
   |                 |     |     |                           |
   |                 |     |     | class NlvQWidget          |
   | display funcs <-|-----|-----|-> private member functions|
   | properties    <-|-----|-----|-> Qt properties           |
   | events        <-|-----|-----|-> Qt signals/slots        |
   | core API cmds <-|-----|-----|-> command()               |
   | Rendering     <.|.....|...  +---------------------------+
   +-----------------+     |  .  +---------------------------+
                           |  .  | FILE: nlvrqt.cpp          |
                           |  .  |                           |
                           |  .  | Qt renderer               |
                           |  .  | (optional, see below)     |
                           |  .  |                           |
                           |  ...|.> QPainter/QPaintDevice   |
                           |     +---------------------------+
                           |
     (nlvcore lib)     C interface     (Qt GUI)
 \endverbatim
 *
 * For the GUI programmer only the \b public and \b protected member function
 * of class NlvQWidget are relevant.
 * Usually you create your own widget either by deriving from this class
 * or by instantiating it directly.
 * There is a test application \c tapp4.cpp (for Qt4) or \c tapp3.cpp (for Qt3)
 * that shows how subclassing works.
 * The test application's code can be used as a template to start with.
 *
 * Communication with the Nlview core is implemented using the
 * \e string-based Nlview [core API].
 * The strings contain commands with arguments
 * for the Nlview core. To send commands to Nlview core, the GUI programmer
 * should use one of the overloaded
 *   \li NlvQWidget::command
 *
 * member functions.
 * Don't miss to read the section in the core manual about how to
 * [deal with lists][lists].
 *
 * \par Example:
 * The code below loads a port into Nlview. It uses the
 * member function NlvQWidget::command to execute the Nlview
 * API core command: [load].
   \code{.cpp}
   bool ok;
   const char* result = nlview->command(&ok, "load", "port", "Q", "input");
   if (!ok)
     qFatal("nlview error message: %s", result);    // contains error msg
   \endcode
 *
 * \par
 * Alternatively, a GUI programmer could attach his own external "foreign"
 * DataBase to Nlview, if he implements the
 * <a href="../nlviewVDI.html">VDI</a>
 * and attaches his implementation to Nlview's Ictrl machine using command
 * <a href="../nlviewIctrl.html#ATTACH">ictrl attach</a>.
 * This requires more integration work, but it enables Nlview to pull in the
 * requested data (incremental navigation or a complete module at once)
 * directly from the foreign DataBase using a higher-level API.
 * There is a VDI reference implementation operating on a test DataBase
 * included in the `tdb' directory; the test application we provide
 * automatically uses this DB if necessary.
 *
 * \anchor signals
 * \par Signals:
 * This class emits Qt signals to notify the GUI programmer about certain
 * changes within the Nlview core. The GUI programmer may connect these signals
 * to corresponding slots to be notified when an event occurs.
 * The following signals are emitted:
 *
 * \par
 * \li NlvQWidget::pageNotify
 * \li NlvQWidget::progressNotify
 * \li NlvQWidget::viewportNotify
 * \li NlvQWidget::selectionNotify
 * \li NlvQWidget::messageOutput
 * \li NlvQWidget::hierarchyDown
 * \li NlvQWidget::hierarchyUp
 * \li NlvQWidget::bindCallback
 *
 * \anchor properties
 * \par Properties:
 * The Properties control colors, fonts, schematic generation details etc.
 * They are modified by the \c set*() and returned by the \c get*() methods.
 * This class has numerous properties that correspond to the Nlview core
 * properties. Changing the Qt properties is equivalent to sending the
 * Nlview core command "property ..." with the function NlvQWidget::command.
 * These Qt properties are provided for convenience.
 * For a detailed description of each property see Nlview [core API]
 * documentation.
 *
 * \anchor rendering
 * \par Rendering:
 * Rendering is the process to actually draw text and graphics
 * (like e.g. lines, polygons, arcs) on a particular output device.
 * Typical output devices are: the screen, a bitmap or the printer.
 * Therefore our Nlview core needs a \c renderer to draw its schematics with.
 * For Qt-style printing support, the renderer must follow the Qt way of
 * "painting" something on a \c QPaintDevice using a \c QPainter.
 * Therefore we provide a Qt renderer.
 *
 * \par
 * The Qt renderer is shipped in source-code (\c nlvrqt.cpp), i.e. it needs
 * to be compiled and linked along with the NlviewQT wrapper files.
 * The Qt renderer is always used for both Qt-style printing
 * (see member function NlvQWidget::Print) and for screen rendering.
 *
 * \par
 * An alternative way of printing Nlview schematics is using
 * the Nlview API command [print].
 *
 * \anchor moc
 * \par MOC:
 * Finally, don't forget to invoke Qt's meta-object-compiler \c moc for each
 * source file containing the \c Q_OBJECT macro;
 * otherwise you might get strange unresolved symbols during linking stage.
 * In short: invoking \c moc on the .h/.cpp files will produce other .cpp
 * files that you compile and link with your application or library.
 * The Qt documentation explains moc'ing in details.
 * 
 * \author Jochen Roemmler
 */
class CLASS_EXPORT NlvQWidget : public QWidget
{

	Q_OBJECT

#ifndef SKIP_DOCUMENTATION
	Q_PROPERTY( QString logfile READ getLogfile WRITE setLogfile )
#endif // SKIP_DOCUMENTATION

	Q_ENUMS ( PrintMode )
	Q_ENUMS ( PrintOrientation )
	Q_ENUMS ( PrintScale )
	Q_ENUMS ( OType )

 public:

	// ******************************************************************
	// The following functions are vital if you want to customize
	// and use the NlviewQT wrapper widget in your application:
	// ******************************************************************

	/*!
	 * \brief
	 *   Constructor for a NlvQWidget object.
	 *
	 * [license]: ../nlviewCore.html#COMMAND_license "Core API: license"
	 *
	 * This will create a new NlviewQT widget with default settings.
	 *
	 * Do not forget to \e license each NlvQWidget object with the
	 * [license] command.
	 * For example, run this code in your application's start-up code
	 * to license Nlview:
	 *
	   \code{.cpp}
	   // assume "cookie" has been received before,
	   // either by calling licrypt() or NlviewCookie().
	   const char* widgetname = "MyNlview";
	   bool ok;
	   const char* result;
	   result = nlview->command(&ok, "license", widgetname, cookie);
	   if (!ok)
	     printf("license error: %s\n", result);
	   \endcode
	 *
	 * \param[in] parent Pointer to a Qt parent widget or NULL.
	 * \param[in] f      Qt window flags.
	 * \param[in] qtrend 2: Enable perfect font scaling (experimental),
	 *                  else disable perfect font scaling (default).
	 * \returns New NlviewQT instance.
	 */
 	NlvQWidget(QWidget*	parent,
#if QT_VERSION >= 0x050700				// Qt5.7+
		   Qt::WindowFlags f = Qt::WindowFlags(),
#elif QT_VERSION >= 0x050000				// Qt5..5.7
		   Qt::WindowFlags f = Qt::Widget,
#else							// Qt2..4
		   Qt::WFlags	f = 0,
#endif
		   int		qtrend = 1);

	/*!
	 * \brief
	 *   Destructor for a NlvQWidget object.
	 *
	 * This destructor frees all memory which has been allocated
	 * by this object.
	 *
	 * \note The scrollbar objects will \b not be freed, because
	 * NlvQWidget does not have ownership (see #setHScrollBar for details).
	 */
	virtual ~NlvQWidget();

	/*!
	 * \brief
	 *   Splits the passed string into its elements.
	 *
	 * [lists]: ../nlviewCore.html#OVERVIEW_LIST "how to deal with lists"
	 *
	 * \par Note:
	 *   This function is deprecated, please use helper class
	 *   NlviewList instead.
	 *
	 * The number of elements is returned in \c argc.
	 * The return value of the function holds a list of
	 * \c const \c char* elements.
	 * This function complies with the tcl rules for building [lists].
	 *
	 * The memory for the returned list is allocated and managed by the
	 * Nlview core. Therefore, it has to be freed by calling #freeList
	 * afterwards.
	 *
	 * #splitList is the counterpart of #mergeList, i.e.
	 * if you split a string A into a list that is later merged, you'll
	 * get the original (contents of) string A.
	 *
	 * \anchor lists
	 * \par Notes on dealing with lists:
	 * Many API commands require [lists]
	 * as arguments or have a list as a return value.
	 * Therefore, we provide service functions and classes that
	 * can handle such lists properly:
	 *
	 * \par
	 * \li #splitList, NlviewList : splits a list into its elements
	 * \li #freeList : frees memory from #splitList call
	 * \li #mergeList : concatenate elements into a single string
	 * \li #freeMerged : frees memory from #mergeList call
	 *
	 * \par
	 * Here is a typical usage example for #splitList and
	 * #freeList :
	   \code{.cpp}
	   QString MyNlvQWidget::getCurrentModule()
	   {
	     bool ok;
	     const char* result = command( &ok, "module" );
	     if (!ok) return "";
	     int argc;
	     const char** list  = splitList( result, &argc );
	     QString curmod( argc > 0 ? list[0] : "" );
	     freeList( list );
	     return curmod;
	   }
	   \endcode
	 *
	 * \par
	 * Equivalently, you can use helper class NlviewList :
	   \code{.cpp}
	   QString MyNlvQWidget::getCurrentModule()
	   {
	     bool ok;
	     NlviewList result = command( &ok, "module" );
	     return (ok && result.split() > 0) ? result[0] : "";
	   }
	   \endcode
	 *
	 * \par
	 * Here is a typical usage example for #mergeList and
	 * #freeMerged :
	   \code{.cpp}
	   bool MyNlvQWidget::centerAtInst(const char* instname)
	   {
	     // create a Nlview object identification for the instance
	     // see: doc/nlviewCore.html#IDENTIFICATION
	     const char* argv[2];
	     argv[0] = "inst";
	     argv[1] = instname;
	     char* instOID = mergeList( 2, argv );
	     bool ok;
	     command( &ok, "center", instOID );
	     freeMerged( instOID );
	     return ok;
	   }
	   \endcode
	 *
	 * \par
	 * Equivalently, you can use helper class NlviewArgs :
	   \code{.cpp}
	   bool MyNlvQWidget::centerAtInst(const char* instname)
	   {
	     NlviewArgs instOID;
	     instOID << "inst" << instname;
	     args << "center" << instOID;    // populate args object
	     bool ok;
	     command( &ok );                 // execute args
	     return ok;
	   }
	   \endcode
	 *
	 * \param[in]  string  the string to split into its elements.
	 * \param[out] argc    pointer to \c int which will hold the list size.
	 * \returns a list of strings; one for each element in the list.
	 *
	 * \sa #splitList, #freeList, #mergeList, #freeMerged,
	 * \sa NlviewList, NlviewArgs
	 */
	static const char** splitList(const char* string, int* argc);

	/*!
	 * \brief
	 *   Joins the elements in the given list of strings.
	 *
	 * [lists]: ../nlviewCore.html#OVERVIEW_LIST "how to deal with lists"
	 *
	 * This function complies with the tcl rules for building [lists], e.g.
	 * each element is separated by a space character; if one of the
	 * elements contains a space character itself, the element will be
	 * surrounded by curly braces...
	 *
	 * The memory of the returned string is allocated and managed by the
	 * Nlview core. Therefore, it has to be freed with #freeMerged
	 * afterwards.
	 *
	 * #mergeList is the counterpart of #splitList, i.e.
	 * if you merge a list A into a string that is later split, you'll
	 * get the original list A.
	 *
	 * For an example, see #splitList.
	 *
	 * \param[in] argc  The number of elements in the string list.
	 * \param[in] argv  The list of strings that should be concatenated.
	 * \returns The concatenated list elements in a single string.
	 *
	 * \sa #freeMerged
	 */
	static char* mergeList(int argc, const char** argv);

	/*!
	 * \brief
	 *   This function frees the memory which has previously been
	 *   allocated by a call to #splitList.
	 *
	 * For an example, see #splitList.
	 *
	 * \param[in] argv  The array of strings which has previously been
	 *                  returned by a call to #splitList.
	 * \sa #splitList
	 */
	static void freeList(const char** argv);

	/*!
	 * \brief
	 *   This function frees the memory which has previously been
	 *   allocated by a call to #mergeList.
	 *
	 * For an example, see #splitList.
	 *
	 * \param[in] string  The string which has previously been returned
	 *                    by a call to #mergeList.
	 * \sa #mergeList
	 */
	static void freeMerged(char* string);

	/*! 
	 * \brief
	 *   Execute a command with up to 5 arguments and return the result.
	 *
	 * [core API]: ../nlviewCore.html               "Nlview Core API"
	 *
	 * Some commands can be long lasting.
	 * Thus the application will not respond to any events.
	 * However, Nlview periodically sends a notification message (signal
	 * #progressNotify) that could at least process any pending
	 * paint events or allow the GUI to call #cancel to interrupt this
	 * command.
	 *
	 * A maximum of 5 arguments is supported by this function.
	 * If you need to pass more than 5 arguments,
	 * choose the more general #command(bool*,int,const char**) function.
	 *
	 * \note
	 *	This is the master command function to access Nlview's
	 *	[core API].
	 *
	 * \param[out] ret If the command could be processed successfully,
	 *                  \c true is stored, \c false otherwise.
	 * \param[in] cmd   The command to be executed by the Nlview core.
	 * \param[in] arg1  The 1st command argument (optional).
	 * \param[in] arg2  The 2nd command argument (optional).
	 * \param[in] arg3  The 3rd command argument (optional).
	 * \param[in] arg4  The 4th command argument (optional).
	 * \param[in] arg5  The 5th command argument (optional).
	 * \returns The result of the command
	 *		(or the error message, if \a ret is set to \c false).
	 *		This result string is valid until
	 *		the next call of #command or #commandLine.
	 *
	 * \sa #command(bool*,int,const char**), #progressNotify, #cancel
	 */
	const char* command(bool* ret, const char* cmd,
			    const char* arg1 = 0,
			    const char* arg2 = 0, 
			    const char* arg3 = 0,
			    const char* arg4 = 0,
			    const char* arg5 = 0);

	/*! 
	 * \brief
	 *   Execute a command with an arbitrary number of arguments.
	 *
	 * [core API]: ../nlviewCore.html               "Nlview Core API"
	 *
	 * This is an overloaded member function that
	 * behaves exactly like the #command function.
	 * The only difference is, that it takes an arbitrary number
	 * (\c argc) of arguments in (\c argv).
	 *
	 * \note
	 *	This is the master command function to access Nlview's
	 *	[core API].
	 *
	 * \param[out] ret If the command could be processed successfully,
	 *                  \c true is stored, \c false otherwise.
	 * \param[in] argc The number of strings in argv
	 * \param[in] argv The command (in \c argv[0]) and the list of
	 *		arguments passed to the Nlview core.
	 *		The pointers in argv may be shuffled around (but the
	 *		strings themselves are of cause not touched).
	 * \returns The result of the command
	 *		(or the error message, if \a ret is set to \c false).
	 *		This result string is valid until
	 *		the next call of #command or #commandLine.
	 *
	 * \sa #command(bool*,const char*,const char*,const char*,const char*,const char*,const char*), #progressNotify, #cancel
	 */
	const char* command(bool* ret, int argc, const char** argv);


	/*! 
	 * \brief
	 *   Execute the command, that is stored in the public #args.
	 *
	 * This is an overloaded member function, provided for convenience.
	 * It behaves essentially like the #command(bool*,int,const char**)
	 * function.
	 * The main difference is, that it operates on the argument vector
	 * that has been (incrementally) populated before using the built-in
	 * public #args object.
	 * After execution of the command the built-in #args object will
	 * be reset (i.e. empty again) - ready to receive new arguments.
	 *
	 * \param[out] ret If the command could be processed successfully,
	 *                  \c true is stored, \c false otherwise.
	 * \returns The result of the command
	 *		(or the error message, if \a ret is set to \c false).
	 *		This result string is valid until
	 *		the next call of #command or #commandLine.
	 *
	 * \sa #command(bool*,NlviewArgs&), #command, #args, NlviewArgs
	 */
	const char* command(bool* ret);

	/*! 
	 * \brief
	 *   Execute the command from the given NlviewArgs.
	 *
	 * This is an overloaded member function, provided for convenience.
	 * It behaves like the #command(bool*) function
	 * but it is executing the given NlviewArgs instead of
	 * the built-in #args object.
	 * After execution of the command the passed \c NlviewArgs object will
	 * be reset (i.e. empty again) - ready to receive new arguments.
	 *
	 * \param[out] ret If the command could be processed successfully,
	 *                  \c true is stored, \c false otherwise.
	 * \param[in] a The command with arguments to be processed -
	 *		see NlviewArgs.
	 * \returns The result of the command
	 *		(or the error message, if \a ret is set to \c false).
	 *		This result string is valid until
	 *		the next call of #command or #commandLine.
	 *
	 * \sa #command(bool*), #command, NlviewArgs
	 */
	const char* command(bool* ret, NlviewArgs& a);

	/*! 
	 * \brief
	 *   Execute a command line (command with space-separated arguments).
	 *
	 * This member function behaves essentially like the
	 * #command(bool*,int,const char**)
	 * except that the command and arguments are passed as one string
	 * (as if they have been concatenated with #mergeList before).
	 *
	 * For performance reasons, you should
	 * prefer #command() over commandLine, if possible.
	 * (Internally, commandLine calls #mergeList, 
	 * #command(bool*,int,const char**) and then #freeList).
	 * The typical usage of commandLine is for executing a
	 * command line from a user console window or from a file.
	 *
	 * \note
	 *   The \a cmdline is expected to be a space-separated list
	 *   of command and arguments.  If one of the arguments include
	 *   spaces or special characters, then those characters need to
	 *   be masked like #mergeList does.
	 *
	 * \param[out] ret If the command could be processed successfully,
	 *                  \c true is stored, \c false otherwise.
	 * \param[in] cmdline  The command line to be evaluated by
	 *		the Nlview core.
	 * \returns The result of the evaluation (or error message).
	 *		This result string is valid until
	 *		the next call of commandLine or #command.
	 *
	 * \sa #command, #mergeList, #freeMerged, #progressNotify, #cancel
	 */
	const char* commandLine(bool* ret, const char* cmdline);

	/*!
	 * \brief
	 *   Sets the horizontal scrollbar object to be used by this widget.
	 *
	 * \par General notes on handling scrollbars:
	 * This class does not provide scrollbars automatically:
	 * You explicitly need to create, display and set a horizontal
	 * and/or vertical scrollbar object using the functions #setHScrollBar
	 * and #setVScrollBar, respectively. This, of course, includes adding
	 * the scrollbar object to the Qt widget hierarchy.
	 * If set, Nlview can update these scrollbars whenever
	 * the schematic content changes programmatically in size, position or
	 * scale and the user is able to manipulate the current viewport using
	 * the scrollbars' controls.
	 * If you need to delete the scrollbar objects while they are still
	 * attached (before the destructor of this class runs), make sure
	 * you first call #setHScrollBar and #setVScrollBar with a \c NULL
	 * pointer to disconnect these scrollbar objects again.
	 *
	 * \param[in] scrollBar  The horizontal scrollbar object to use.
	 *
	 * \sa #getHScrollBar, #setVScrollBar, #getVScrollBar
	 */
 	void setHScrollBar(QScrollBar* scrollBar);

	/*!
	 * \brief
	 *   Returns the horizontal scrollbar object from a previous
	 *   call to #setHScrollBar.
	 *
	 * See #setHScrollBar for general notes on handling scrollbars.
	 *
	 * \returns \c NULL or
	 *          the currently attached horizontal scrollbar object.
	 *
	 * \sa #setHScrollBar, #setVScrollBar, #getVScrollBar
	 */
	QScrollBar* getHScrollBar() const { return hScroll; };

	/*!
	 * \brief
	 *   Sets the vertical scrollbar object to be used by this widget.
	 *
	 * See #setHScrollBar for general notes on handling scrollbars.
	 *
	 * \param[in] scrollBar  The vertical scrollbar object to use.
	 *
	 * \sa #getVScrollBar, #setHScrollBar, #getHScrollBar
	 */
	void setVScrollBar(QScrollBar* scrollBar);

	/*!
	 * \brief
	 *   Returns the vertical scrollbar object from a previous
	 *   call to #setVScrollBar.
	 *
	 * See #setHScrollBar for general notes on handling scrollbars.
	 *
	 * \returns \c NULL or
	 *          the currently attached vertical scrollbar object.
	 *
	 * \sa #setVScrollBar, #setHScrollBar, #getHScrollBar
	 */
	QScrollBar* getVScrollBar() const { return vScroll; };


	// ******************************************************************
	// The following access/modifier functions had to be declared
	// because they handle private properties that are not core properties.
	// Therefore, the implementations of these functions do NOT try to
	// access/modify (non-existent) core properties.
	// ******************************************************************

	/*!
	 * \brief
	 *   Turn Nlview's API file logging on or off.
	 *
	 * [logfile]: ../nlviewCore.html#COMMAND_logfile  "Core API: logfile"
	 *
	 * Since all commands passed to Nlview are string-based,
	 * Nlview can easily log all commands into a logfile, e.g. for
	 * debugging.
	 * To start logging into a file, simply pass its filename
	 * as an argument to this function.
	 * This will execute the [logfile] command internally.
	 * To stop logging API commands
	 * into the file, simply call this function with an empty filename.
	 *
	 * \note The file will be overwritten, if it already exists.
	 *
	 * \param fname  Filename of logfile or empty string
	 *
	 * \sa #getLogfile
	 */
	void setLogfile(const QString& fname);

	/*!
	 * \brief
	 *   Returns the name of the current logfile.
	 *
	 * \returns Name of the logfile as set by a prior call to #setLogfile.
	 *
	 * \sa #setLogfile
	 */
	QString getLogfile() const { return logFile; };

	/*!
	 * \brief
	 *   This enum is used for the #Print function.
	 *
	 * The enumerators correspond to the values of the
	 * [print](../nlviewCore.html#PRINTING "Core API: print") command's
	 * [-colormode](../nlviewCore.html#COMMAND_print_colormode
	 *              "choosing the color mode for printing")
	 * option.
	 */
	enum PrintMode {
	    MONO,	/*!< use only two "colors" monochrome mode */
	    COLOR,	/*!< use full color mode */
	    COLORINV,	/*!< use invert-color mode */
	    COLORINV2	/*!< use invert-luminance color mode */
	};

	/*!
	 * \brief
	 *   This enum is used for the #Print function.
	 *
	 * The enumerators correspond to the values of the
	 * [print](../nlviewCore.html#PRINTING "Core API: print") command's
	 * [-scale](../nlviewCore.html#COMMAND_print_scale
	 *              "choosing the scale for printing")
	 * option.
	 */
	enum PrintScale {
	    FULL,	/*!< Fit the full schematic page on the printer page */
	    FULLFIT,	/*!< Fit only schematic drawing on the printer page  */
	    VISIBLE,	/*!< Print only the visible part of the schematic    */
	    VIEW	/*!< Print only current viewport of the schematic    */
	};

	/*!
	 * \brief
	 *   This enum is used for the #Print function.
	 *
	 * The enumerators correspond to the values of the
	 * [print](../nlviewCore.html#PRINTING "Core API: print") command's
	 * [-orientation](../nlviewCore.html#COMMAND_print_orientation
	 *                "choosing the orientation for printing")
	 * option.
	 */
	enum PrintOrientation {
	    AUTO,	/*!< Let Nlview determine the best orientation for
			 * printing. If you're printing a series of pages,
			 * this may result in some schematics being rotated
			 * and some not (depends on each schematic's layout).
			 */
	    LANDSCAPE,	/*!< Print in landscape orientation. */
	    PORTRAIT,	/*!< Print in portrait orientation. */
	    ROTATE,	/*!< Rotate the schematic (not the page!) 90 degrees
			 * counterclockwise (regardless of page's aspect ratio).
			 */
	    NOTROTATE	/*!< Do not rotate the schematic when printing. */
	};


	/*!
	 * \brief
	 *   Prints the current schematic using a \c QPainter that is
	 *   initialized on a \c QPaintDevice.
	 * 
	 * [module]: ../nlviewCore.html#MODULE         "Nlview module"
	 * [npages]: ../nlviewCore.html#COMMAND_npages "Core API: npages"
	 * [print]:  ../nlviewCore.html#PRINTING       "Core API: printing"
	 *
	 * This function prints a single schematic page with the given
	 * \c QPainter. When printing a series of pages, your application
	 * must take care of all the external parameters, like e.g. page-order
	 * (ascending/descending), page-range, number of copies, etc.
	 *
	 * The application must keep track of the pages,
	 * e.g. if the given \c QPainter is initialized on a \c QPrinter
	 * (\c QPaintDevice subclass)
	 * you must call \c QPrinter::newPage() between each two printed
	 * pages to perform a form-feed on that device.
	 * 
	 * \pre
	 *   The \c QPainter must be created by the application which retains
	 *   ownership of that object. The \c QPainter must be initialized
	 *   to operate on a \c QPaintDevice. This can be done either by using
	 *   \c QPainter 's constructor that takes a \c QPaintDevice* or by
	 *   using \c QPainter::begin(QPaintDevice*).
	 *
	 * A \c QPaintDevice can be one of the following classes
	 * (or descendants):
	 *
	 * \li \c QPrinter -
	 *     Create a \c QPrinter object, initialize it and start a
	 *     \c QPainter on it which you pass to this function.
	 *
	 * \li \c QWidget \c QPixmap or \c QImage (Qt>=4) -
	 *     Create a \c QWidget \c QPixmap or \c QImage object and start a
	 *     \c QPainter on it which you pass to this function.
	 *     This is useful for print-preview or snapshot functionality.
	 *     If the fillbg argument is set to \c false or when printing in
	 *     MONO mode, it is necessary to \b initialize the background
	 *     color of the target explicitly, e.g. by calling
	 *     \c QPixmap::fill(const QColor&).
	 *
	 * Additionally, you may customize the printing behavior thru
	 * these arguments: #PrintMode, #PrintScale, #PrintOrientation.
	 * The default arguments would render a colored representation of the
	 * current viewport with auto-rotation mode using the passed \c QPainter
	 * object. The ownership of the \c QPainter
	 * objects is not transferred to Nlview, i.e. you must delete them
	 * manually when finished.
	 *
	 * This function must not be called without a current Nlview [module].
	 *
	 * To print a complete Nlview module (which usually includes several
	 * schematic pages) the application typically performs the following
	 * steps:
	 *
	 * \li Make sure that Nlview has a current [module]
	 * \li Count the number of pages (command [npages])
	 * \li Create a \c QPrinter object and set it up,
	 *      or use a printer dialog to let the user do this.
	 * \li Query the margins from the \c QPrinter object
	 * \li initialize a \c QPainter to start painting on that printer device
	 * \li call this #Print function in a (possibly nested) loop
	 *     to print each page/copy;
	 * \li delete the \c QPrinter and the \c QPainter objects
	 *
	 * \note An alternative way of printing Nlview schematics is using
	 *       the Nlview API command [print].
	 * 
	 * \note The \c pageNum argument is the Nlview schematic page to print.
	 *       The special value -1 is used to print the current page with
	 *       low details (screen/preview mode) and 0 is used to print the
	 *       current page with all details. Values less than -1 are
	 *       rejected. All other (positive) values are taken as page number
	 *       and are printed in high detail mode.
	 * 
	 * \note The return values \c xoff \c yoff and \c zoom are optional.
	 *       If the call was successful (\c true) they store some details;
	 *       the details are a affine transformation from p's coordinate
         *       space to Nlview's internal database coordinate space:
	 *       \c xoff / \c yoff is the offset of Nlview's origin
	 *       from p's origin measured in p's coordinates
         *       (a translation). \c zoom is the zoom factor (a scaling).
	   \verbatim
	   Map Nlview DB coord (dbx,dby) to p's coord (X,Y):
	   -------------------------------------------------
	   X = xoff + (dbx * zoom);
	   Y = yoff + (dby * zoom);
          
           Map p's coord (X,Y) to Nlview DB coord (dbx,dby):
	   -------------------------------------------------
	   dbx = (X - xoff) / zoom;
	   dby = (Y - yoff) / zoom;
	   \endverbatim
	 *
	 * \param[in] p  the \c QPainter to use for painting / printing.
	 * \param[in] printarea the area to paint / print on
	 * \param[in] pageNum the Nlview page number to print (-1, 0 or >= 1)
	 * \param[in] m #PrintMode   the color mode for printing
	 * \param[in] s #PrintScale  the scale to print
	 * \param[in] o #PrintOrientation the schematic orientation
	 *              (not the page orientation)
	 * \param[in] hi \c true to print highlight colors and selection.
	 * \param[out] xoff  horizontal shift of schematic on printout.
	 * \param[out] yoff  vertical shift of schematic on printout.
	 * \param[out] zoom  zoom factor (scale) used for printing.
	 * \param[in] fillbg \c true to fill page with backgroundcolor.
	 * \returns \c true, if successful, \c false otherwise
	 *
	 * \sa #PrintMode, #PrintScale, #PrintOrientation,
	 *     <a href="../nlviewCore.html#PRINTING">print</a>
	 */
	bool Print(QPainter* p,
		   const QRect& printarea,
		   int pageNum, 
		   enum PrintMode        m = COLOR,
		   enum PrintScale       s = VIEW,
		   enum PrintOrientation o = AUTO,
		   bool                  hi = false,
		   int*                  xoff = NULL,
		   int*                  yoff = NULL,
		   double*               zoom = NULL,
		   bool                  fillbg = true);

	/*!
	 * \brief
	 *   Returns the recommended size of this widget.
	 *
	 * See the "sizeHint" property of \c QWidget class for details.
	 * Reimplement this function to return a different recommended size.
	 *
	 * \returns recommended size for the widget; default is (500, 500)
	 */
	virtual QSize sizeHint() const { return QSize( 500, 500 ); };

	/*!
	 * \brief
	 *   Return pointer to internal NlvCore.
	 *
	 * See nlvcore.h for C-level access to NlviewQT's core functions.
	 */
	struct NlvCore* core();

	/*!
	 * \brief
	 *   Register an image to be referred to by
	 *   <a href="../nlviewSymlib.html#imagedsp">imagedsp</a> or
	 *   <a href="../nlviewSymlib.html#button">button</a>.
	 *
	 * The given \c iname must refer to one of the names specified in the
	 * \c imagedsp or \c button entries of a
	 * <a href="../nlviewCore.html#LOADING_symbol">symbol</a>
	 * definition in Nlview's
	 * <a href="../nlviewCore.html#LOADING">netlist</a>.
	 * It may also refer to an image or button name which is created
	 * internally by Nlview (they are prefixed by a @ character).
	 *
	 * \par Note:
	 *   The image pointer must remain valid until this Nlview object is
	 *   deleted. You can tell Nlview whether or not to dispose the
	 *   image object automatically.
	 *
	 * \par Note:
	 *   Registering an already registered image will replace the old one
	 *   - potentially disposing the old image object if allowed to.
	 *
	 * \par Note:
	 *   To release a registered image, call #unregisterImage;
	 *   to release all registered images, call #unregisterImages.
	 *
	 * \param[in] iname unique name of the image
	 * \param[in] img   The QPixmap to register
	 * \param[in] limitscale if \c true, then the scaling is limited to
	 *		    the maximum of QPixmap's natural size to avoid
	 *		    dithering (but will always shrink).
	 * \param[in] free  if \c true (default) ownership of the \c img
	 *                  pointer will be transferred and the destructor of
	 *                  the image may be called at any time.
	 *
	 * \sa
	 *   <a href="../nlviewSymlib.html#imagedsp">imagedsp</a> keyword,
	 *   <a href="../nlviewSymlib.html#imagedsp">button</a> keyword,
	 *   <a href="../nlviewCore.html#LOADING_symbol">symbol</a> loading,
	 *   #unregisterImage, #unregisterImages.
	 */
	void registerImage(const QString& iname, QPixmap* img,
					bool limitscale, bool free=true);

	/*!
	 * \brief
	 *   Register an image to be referred to by
	 *   <a href="../nlviewSymlib.html#imagedsp">imagedsp</a> or
	 *   <a href="../nlviewSymlib.html#button">button</a>.
	 *
	 * This is an overloaded member function.
	 * It behaves exactly like
	 * #registerImage(const QString&,QPixmap*,bool,bool)
	 * but accepts a \c QImage* as argument.
	 *
	 * \par Warning:
	 *   You can only call this overload with Qt 4 or newer!
	 */
	void registerImage(const QString& iname, QImage* img,
					bool limitscale, bool free=true);

	/*!
	 * \brief
	 *   Register an image to be referred to by
	 *   <a href="../nlviewSymlib.html#imagedsp">imagedsp</a> or
	 *   <a href="../nlviewSymlib.html#button">button</a>.
	 *
	 * This is an overloaded member function.
	 * It behaves exactly like
	 * #registerImage(const QString&,QPixmap*,bool,bool)
	 * but accepts a \c QPicture* as argument.
	 *
	 * \par Warning:
	 *   This overload requires Qt 3 or newer!
	 *   QPicture with coordinate transformations applied
	 *   do not seem to work reliably until Qt 4.4.3 or newer.
	 */
	void registerImage(const QString& iname, QPicture* img,
					bool limitscale, bool free=true);

	/*!
	 * \brief
	 *   Remove a previously registered image.
	 *
	 * The given \c iname must refer to an image name that has
	 * been previously registered using one of the #registerImage functions.
	 * This will cause the image to be removed from Nlview's internal
	 * data structures; the image object will only be freed if its
	 * ownership had been transferred to Nlview during image registration.
	 *
	 * \param[in] iname name of a previously registered image
	 *
	 * \sa
	 *   #registerImage, #unregisterImages
	 */
	void unregisterImage(const QString& iname);

	/*!
	 * \brief
	 *   Remove all previously registered images.
	 *
	 * This method will remove all previously registered images
	 * from Nlview's internal data structures in one go.
	 * The image objects will only be freed if their
	 * ownership had been transferred to Nlview during image registration.
	 *
	 * \sa
	 *   #registerImage, #unregisterImage
	 */
	void unregisterImages();


 public slots:
	/*!
	 * \brief
	 *   Interrupts a busy Nlview core.
	 *
	 * Some commands sent to Nlview through one of the
	 * #command functions
	 * may possibly take a longer time.
	 * Calling this function will stop the execution of that
	 * command to the next possible state
	 * and return control to the caller again.
	 *
	 * Because the #command functions do not run asynchronously
	 * (non-multi-threaded) your application (GUI) is usually
	 * blocked during the command execution. To work around this
	 * limitation, the widget periodically sends a #progressNotify
	 * signal to the GUI while executing a long-lasting command.
	 * You could connect a slot to signal #progressNotify.
	 * In this slot you may check if the user wants to stop the
	 * command and call this #cancel function.
	 *
	 * Usually you want to implement \e user-interactive cancellation;
	 * e.g. to allow the user to press a "Cancel" button:
	 *
	   \code{.cpp}
	   QPushButton* c = new QPushButton("Cancel", this);
	   connect(c, SIGNAL(clicked()), nlview, SLOT(cancel()));
	   \endcode
	 *
	 * To process such user input events you have to flush the Qt event
	 * queue in a slot connected to #progressNotify.
	 *
	 * \sa #progressNotify, #command
	 */
 	void cancel();


 public:
	/*!
	 * \brief
	 *   A ready-to-use helper NlviewArgs object for creating
	 *   well-formatted argument \ref lists for subsequent calls to
	 *   #command(bool*) method.
	 *
	 * This object is a public class member for convenience;
	 * its internal memory can be reused across
	 * multiple calls to #command(bool*).
	 *
	 * \par Example 1:
	 *
	   \code{.cpp}
	   nlview->args << "module" << "new" << "my module" << "my view";
	   nlview->command(&ok);
	  
	   nlview->args << "load" << "port" << "funny port" << "input";
	   nlview->command(&ok);
	  
	   nlview->command(&ok, "show");
	   \endcode
	 *
	 * \par Example 2:
	 *
	 * As an alternative to using this built-in member, you can use
	 * your own NlviewArgs object and call 
	 * #command(bool*,NlviewArgs&) method instead.
	 *
	   \code{.cpp}
	   NlviewArgs instID;
	   instID << "inst" << "my instance";
	  
	   NlviewArgs myargs;
	   myargs << "selection" << "-itemized" << instID;
	   nlview->command(&ok, myargs);
	   \endcode
	 * \sa NlviewList, NlviewArgs
	 */
	NlviewArgs args;


	/*!
	 *   This enum can be used to interpret the flags in the details
	 *   argument of the #pageNotify signal.
	 */
	enum PageNotifyDetails {
	    PageNotifyDPage         = 0x0001, /*!< Current Page switched */
	    PageNotifyDPageCnt      = 0x0002, /*!< Page Count changed */
	    PageNotifyDModule       = 0x0004, /*!< Current Module switched */
	    PageNotifyDCleared      = 0x0008, /*!< Schematic cleared */
	    PageNotifyDLayout       = 0x0010, /*!< Schematic Layout modified */
	    PageNotifyDMouse        = 0x0020, /*!< Source is a Mouse Event */
	    PageNotifyDMagnifyClose = 0x0040, /*!< Magnify Window was closed */
	    PageNotifyDMagnifyOpen  = 0x0080  /*!< Magnify Window was opened */
	};

 signals:
        // ******************************************************************
	// The following signals are emitted by the NlviewQT wrapper.
	// Connect these signals to your slots to be notified whenever an
	// appropriate event emits one of the signals below:
	// ******************************************************************

	/*!
	 * \brief
	 *   Signal to notify about a change of Nlview's schematic page.
	 *
	 * A simple usage for this would be: connect this signal to a slot
	 * which updates a label in the application that displays the 
	 * current Nlview page number and page count.
	 *
	 * \param[in] details an or-combination of the #PageNotifyDetails flags
	 *	      define the reasons why this event is sent.
	 * \param[in] page_no \c 0 or the current page number.
	 * \param[in] pagecnt \c 0 or the total number of pages.
	 *
	 * \sa
	 *   <a href="../nlviewCore.html#CALLBACK_pagenotify">pagenotify</a>
	 *   <a href="../nlviewCore.html#OVERVIEW_CALLBACK">Nlview callbacks</a>
	 */
	void pageNotify(unsigned details, int page_no, int pagecnt);

	/*!
	 * \brief
	 *   Signal to notify about a change in Nlview's viewport.
	 *
	 * \c (left,top)-(right,bot) defines the currently visible
	 * rectangular area in Nlview's internal database coordinates.
	 * The aspect ratio of this rectangle should exactly match the
	 * widget's window aspect ratio.
	 *
	 * \param[in] left The database (unzoomed) coordinate of the
         *            left window edge.
	 * \param[in] top The database (unzoomed) coordinate of the
	 *            top window edge.
	 * \param[in] right The database (unzoomed) coordinate of the
	 *            right window edge.
	 * \param[in] bot The database (unzoomed) coordinate of the
	 *            bottom window edge.
	 *
	 * \sa
	 *   <a href="../nlviewCore.html#OVERVIEW_CALLBACK">Nlview callbacks</a>
	 *   <a href="../nlviewCore.html#COMMAND_zoom">scrollpos</a>
	 */
	void viewportNotify(long left, long top, long right, long bot);

	/*!
	 * \brief
	 *   Signal to notify about a progress of a long running Nlview command
	 *
	 * This Qt signal is emitted to notify a progress in a
	 * long running #command. You can connect this
	 * signal to one of your slots in order to visualize the progress
	 * e.g. with a progress bar.
	 *
	 * However, since the #command functions
	 * run synchronously, your GUI will not be able to process paint
	 * events or user input while the command is running unless you
	 * explicitly tell the Qt framework to do so:
	 *
	   \code{.cpp}
	   qApp->processEvents(); // flush Qt event queue
	   \endcode
	 *
	 * This allows the GUI to respond to e.g. paint events
	 * caused by your progress bar updates.
	 *
	 * \warning
	 *   Nlview is \e not reentrant.
	 *   Flushing the event queue may also process user input events,
	 *   like mouse-clicks, which may result in unexpected calls to the
	 *   Nlview core while it is processing a command.
	 *   Since Qt 3.1 you have the possibility to filter the events that
	 *   are processed. E.g. if you don't want to process any user input
	 *   events (but paint events) you can use the following code instead:
	     \code{.cpp}
	     // Qt3.1+
	     QEventLoop* loop = qApp->eventLoop();
	     loop->processEvents(QEventLoop::ExcludeUserInput);
	  
	     // Qt4.0+
	     qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
	     \endcode
	 *
	 * \param[in] cnt A "heartbeat"; number of #progressNotify signals
	 *                emitted since command has been started.
	 *                The special number \c -1 denotes the last call.
	 * \param[in] percent The percentage of progress from
	 *                    0.0 to 100.0 (a rough guessing)
	 * \param[in] msg Internal message about what is currently being done.
	 *
	 * \sa #command,
	 *   <a href="../nlviewCore.html#OVERVIEW_CALLBACK">Nlview callbacks</a>
	 */
	void progressNotify(int cnt, double percent, const char* msg);

	/*!
	 * \brief
	 *   Signal to notify about a (possible) change in current
	 *   <a href="../nlviewCore.html#SELECTION">Nlview selection</a>.
	 *
	 * A usage example would be: connect this signal to a slot that
	 * queries Nlview about its current
	 * <a href="../nlviewCore.html#SELECTION">selection</a>
	 * and displays the updated information in a small window
	 * (maybe including addition information from your DataBase),
	 * or displays the number of selected objects, etc.
	 *
	 * \par Note:
	 *   Be careful not to execute a command that triggers another
	 *   selectionNotify signal in a slot connected to this signal;
	 *   this might end up in infinite recursion - and finally a crash.
	 *
	 * \param[in] length the current length of Nlview's selection list;
	 *                   equivalent to result of command
	 *<a href="../nlviewCore.html#COMMAND_selection_query">selection_len</a>
	 *
	 * \sa
	 *   <a href="../nlviewCore.html#SELECTION">Nlview selection</a>,
	 *   <a href="../nlviewCore.html#OVERVIEW_CALLBACK">Nlview callbacks</a>
	 */
 	void selectionNotify(unsigned length);

	/*!
	 * \brief
	 *   Signal to notify about error conditions and warning messages
         *   generated by the Nlview core.
	 *
	 * Usually, you connect this signal
	 * to a slot that prints the message into a logging
	 * widget or file.
	 *
	 * \param[in] id   The kind of message, for example "ERR" or "WAR"
	 * \param[in] txt  The detailed message
	 *
	 * \sa
	 *   <a href="../nlviewCore.html#OVERVIEW_CALLBACK">Nlview callbacks</a>
	 */
	void messageOutput(const char* id, const char* txt);

	/*!
	 * \brief Signal to notify that action \e navigate of the
	 *   <a href="../nlviewCore.html#COMMAND_bind">bind</a> command
	 *   has been triggered.
	 *
	 * This signal is particularly interesting
	 * if you're planning to implement hierarchical navigation.
	 * A typical response to this signal would be to load
	 * the sub-module's contents into Nlview (if it is not a primitive
	 * instance).
	 *
	 * \param[in] instName  Name of the instance to go down to.
	 *
	 * \sa #hierarchyUp,
	 *   <a href="../nlviewCore.html#OVERVIEW_CALLBACK">Nlview callbacks</a>
	 */
	void hierarchyDown(const char* instName);

	/*!
	 * \brief
	 *   Signal to notify that action \e goup of the
	 *   <a href="../nlviewCore.html#COMMAND_bind">bind</a> command
	 *   has been triggered.
	 *
	 * This signal is particularly interesting
	 * if you're planning to implement hierarchical navigation.
	 * A typical response to this signal would be to load
	 * the parent-module's contents into Nlview (if there is one).
	 *
	 * \sa #hierarchyDown,
	 *   <a href="../nlviewCore.html#OVERVIEW_CALLBACK">Nlview callbacks</a>
	 */
	void hierarchyUp();

	/*!
	 * \brief
	 *   Signal to notify that action \e callback of the
	 *   <a href="../nlviewCore.html#COMMAND_bind">bind</a> command
	 *   has been triggered.
	 *
	 * This Qt signal is emitted whenever the user has triggered a
	 * mouse event (e.g. a stroke, click or wheel-turn) that has been
	 * <a href="../nlviewCore.html#COMMAND_bind">bound</a>
	 * to Nlview action \e callback.
	 *
	 * \param[in] bindingType  the bind event type which sent this signal
	 * \param[in] downX  the x position when the mouse button was clicked
	 * \param[in] downY  the y position when the mouse button was clicked
	 * \param[in] upX  the x position when the mouse button was released
	 * \param[in] upY  the y position when the mouse button was released
	 * \param[in] wdelta  the mouse wheel delta in 1/8th of a degree
	 *                    (only for wheel-type bindings, else 0)
	 * \param[in] oid  the
         *	<a href="../nlviewCore.html#IDENTIFICATION">object_id</a>
	 *	at the current mouse location, or NULL if not available (see
	 *	<a href="../nlviewCore.html#CALLBACK_bindcallback">details</A>).
	 *
	 * \sa
	 *   <a href="../nlviewCore.html#OVERVIEW_CALLBACK">Nlview callbacks</a>
	 */
	void bindCallback(const char* bindingType,
			  int downX, int downY,
			  int upX,   int upY,
			  int wdelta, const char* oid);

#ifndef SKIP_DOCUMENTATION
	/*!
	 * \internal
	 * \brief
	 *   Signal to notify about a change in the current Nlview schematic
	 *   page (NlviewMinimap only).
	 *
	 * \param[in] scrollr The current scroll region of Nlview,
	 *                    which is the total scrollable area in
	 *                    zoomed DataBase coordinates
	 *                    (== WIDGET coordinates).
	 */
	void mapChanged(const QRect& scrollr);

	/*!
	 * \internal
	 * \brief
	 *   Signal to notify about a change in the current Nlview schematic
	 *   page's visible area (== viewport) (NlviewMinimap only).
	 *
	 * The viewport is defined by the current window size
	 * and Nlview scroll offset and is measured in zoomed DataBase
	 * coordinates (== WIDGET coordinates).
	 *
	 * \param[in] viewport The new viewport rectangle of Nlview.
	 * \param[in] origin The true scroll offset, which may differ from
	 *                   viewport's top-left corner due to internal
	 *                   widget borders, etc.
	 */
	void viewportChanged(const QRect& viewport, const QPoint& origin);

	/*!
	 * \internal
	 * \brief
	 *   Signal to notify about a change in the list of registered images.
	 */
	void imageMapChanged();
#endif



 protected:
	// ******************************************************************
	// The following protected member functions are for informing the
	// Nlview core about external (GUI) events. If you want to change
	// the default behavior of these functions, you must overwrite these
	// virtual functions. Don't forget to call the base class' member.
	// ******************************************************************

	/*!
	 * \brief
	 *   Reimplemented from QWidget class to receive mouse press events
	 *   for this widget.
	 *
	 * The Qt framework calls this mouse handler
	 * whenever a user pressed a mouse button on the Nlview widget.
	 * In this implementation we tell Nlview about the mouse press event;
	 * this enables Nlview to respond to this mouse event,
	 * e.g. selecting the component at mouse cursor position.
	 *
	 * If you want to implement your own behavior upon mouse press events,
	 * (e.g. to implement Drag-and-Drop support or for a popup menu to
	 * appear in the GUI) you need to reimplement
	 * this function; but do not forget to call the base class' member
	 * for every unhandled situation in your reimplementation.
	 *
	 * Reimplementation example:
	   \code{.cpp}
	   void MyNlvQWidget::mousePressEvent(QMouseEvent* ev)
	   {
	     if ( ev->button() == Qt::RightButton ) {
	       // open a small popup menu when right mouse button is pressed
	       popupMenu->popup(mapToGlobal( ev->pos()) );
	     } else {
	       // unhandled: call base class' member
	       // perhaps NlvQWidget knows what to do with that event
	       NlvQWidget::mousePressEvent( ev );
	     }
	   }
	   \endcode
	 *
	 * \param[in] ev The mouse event to process.
	 *
	 * \sa #mouseReleaseEvent, #mouseDoubleClickEvent, #mouseMoveEvent,
	 * \sa #wheelEvent
	 */
	virtual void mousePressEvent(QMouseEvent* ev);

	/*!
	 * \brief
	 *   Reimplemented from QWidget class to receive mouse release events
	 *   for this widget.
	 *
	 * The Qt framework calls this mouse handler
	 * whenever a user released a mouse button on the Nlview widget.
	 * In this implementation we tell Nlview about the mouse release event;
	 * this enables Nlview to respond to this mouse event,
	 * e.g. to execute a mouse-stroke action.
	 *
	 * If you want to implement your own behavior upon mouse release
	 * events, you need to reimplement this function;
	 * but do not forget to call the base class' member
	 * for every unhandled situation in your reimplementation.
	 * This function rarely needs to be overwritten.
	 *
	 * \param[in] ev The mouse event to process.
	 *
	 * \sa #mousePressEvent, #mouseDoubleClickEvent, #mouseMoveEvent,
	 * \sa #wheelEvent
	 */
	virtual void mouseReleaseEvent(QMouseEvent* ev);

	/*!
	 * \brief
	 *   Reimplemented from QWidget class to receive mouse double click
	 *   events for this widget.
	 *
	 * The Qt framework calls this mouse handler
	 * whenever a user double-clicked a mouse button on the Nlview widget.
	 * In this implementation we tell Nlview about the mouse double-click
	 * event; this enables Nlview to respond to this mouse event,
	 * e.g. to execute a mouse-stroke action.
	 *
	 * If you want to implement or add your own behavior upon mouse
	 * double-click events, you need to reimplement this function;
	 * but do not forget to call the base class' member
	 * for every unhandled situation in your reimplementation.
	 *
	 * Typical reasons for reimplementation are:
	 *
	 * \li incremental navigation using Ictrl: call "ictrl more ..."
	 * \li interactive hierarchical navigation
	 * \li toggling autohide of unconnected instance pins at symbols
	 * \li ...
	 *
	 * \param[in] ev The mouse event to process.
	 *
	 * \sa #mousePressEvent, #mouseReleaseEvent, #mouseMoveEvent,
	 * \sa #wheelEvent
	 */
	virtual void mouseDoubleClickEvent(QMouseEvent* ev);

	/*!
	 * \brief
	 *   Reimplemented from QWidget class to receive mouse motion
	 *   events for this widget.
	 *
	 * The Qt framework calls this mouse handler
	 * whenever a user moved the mouse over the Nlview widget.
	 *
	 * \note
	 *   This also depends on \c QWidgets 's \e mouseTracking property.
	 *
	 * In this implementation we tell Nlview about the mouse motion
	 * event; this enables Nlview to respond to this mouse event,
	 * e.g. to draw the proper mouse-strokes (if button is pressed)
	 * or to move the schematic during panning.
	 *
	 * If you want to implement or add your own behavior upon mouse
	 * motion events (e.g. for implementing Drag-and-Drop support),
	 * you need to reimplement this function;
	 * but do not forget to call the base class' member
	 * for every unhandled situation in your reimplementation.
	 *
	 * \param[in] ev The mouse event to process.
	 *
	 * \sa #mousePressEvent, #mouseReleaseEvent, #mouseDoubleClickEvent,
	 * \sa #wheelEvent
	 */
	virtual void mouseMoveEvent(QMouseEvent* ev);

	/*!
	 * \brief
	 *   Reimplemented from QWidget class to receive (mouse) wheel
	 *   events for this widget.
	 *
	 * The Qt framework calls this handler
	 * whenever a user turned the wheel (typically part of his mouse)
	 * over the Nlview widget.
	 *
	 * In this implementation we tell Nlview about the mouse wheel
	 * event; this enables Nlview to trigger actions bound to this
	 * wheel event, e.g. to scroll or zoom the schematic.
	 *
	 * If you want change this behavior, you can either use the
	 * Nlview <a href="../nlviewCore.html#BINDINGS">bind</a>
	 * command or you can overwrite this function;
	 * but do not forget to call the base class' member
	 * for every unhandled situation in your reimplementation or
	 * the built-in \c wheel type bindings will stop working.
	 *
	 * \param[in] ev The wheel event to process.
	 *
	 * \sa #mousePressEvent, #mouseReleaseEvent, #mouseDoubleClickEvent,
	 * \sa #mouseMoveEvent
	 */
	virtual void wheelEvent(QWheelEvent* ev);

	/*!
	 * \brief
	 *   Reimplemented from QWidget class to receive paint events
	 *   for this widget.
	 *
	 * This function initiates the actual rendering
	 * of Nlview schematics if necessary.
	 * It uses a double-buffering technology for flicker-free
	 * updates.
	 * This function rarely needs to be reimplemented; however, if so,
	 * the derived function's implementation should look similar to this:
	 *
	   \code{.cpp}
	   void MyNlvQWidget::paintEvent(QPaintEvent* ev) {
	      NlvQWidget::paintEvent(ev); // draw the schematic first
	      QPainter p(this);
	      p.setPen(Qt::white);
	      p.drawText(5,height()-5, "text on top of schematic");
	   }
	   \endcode
	 *
	 * \param[in] ev The paint event to process.
	 */
	virtual void paintEvent(QPaintEvent* ev);

	/*!
	 * \brief
	 *   Reimplemented from QWidget class to receive resize events
	 *   for this widget.
	 *
	 * This internally resizes the double-buffer, updates the
	 * scrollbars and redraws the schematic.
	 * This function rarely needs to be reimplemented.
	 *
	 * \param[in] ev The resize event to process.
	 */
	virtual void resizeEvent(QResizeEvent* ev);

	/*!
	 * \brief
	 *   Overwrite this function to customize text eliding for each label.
	 *
	 * To avoid text overlaps or overwide schematics,
	 * Nlview can elide text labels exceeding certain character limits.
	 * This behavior is globally controlled by a configure property called
	 * <a href="../nlviewCore.html#PROP_elidetext">elidetext</a>.
	 * If active, Nlview automatically elides long labels using its own
	 * policy. If you want to implement a custom text eliding policy,
	 * you can overwrite this function and return your elided text in
	 * the pre-allocated \c result buffer.
	 * But to minimize overhead, this custom function will only be called
	 * after being explicitly enabled with
	 * \code{.cpp} setCustomEliding(true); \endcode
	 * 
	 * Expected behavior:
	 * The un-elided label is passed as \c text parameter.
	 * The string length of \c text is \c len, which is also the maximum
	 * string length the pre-allocated \c result buffer can hold.
	 * The elided text must be copied into the \c result buffer
	 * according to your custom eliding rules.
	 * The input parameter \c maxlen tells you how many characters you
	 * should copy, because it's the (guessed) maximum number of
	 * characters still fitting into the spot without overlappings.
	 * If you copy more than \c maxlen characters into \c result,
	 * then text overlappings might occur.
	 * Make sure \c result is properly NULL-terminated.
	 * \c maxlen will always be less than \c len, otherwise eliding is
	 * not required at all for that label and consquently
	 * #getCustomElidedText will not be called.
	 *
	 * Some additional information about the text to be elided is passed
	 * along to this function to increase flexibility:
	 * \li \c otype carries an identifier of the object type
	 *     the label is associated with;
	 *     it is a subset of enum NlviewList::OType.
	 * \li \c aname is the name of the
	 *     <a href="../nlviewCore.html#ATTRIBUTES_display">
	 *     Display Attribute</a> (attribute type)
	 *     from which this label originates
	 *     (e.g. "@name" or "@cell").
	 *
	 * \par Notes:
	 * \li Text eliding will happen frequently and is time critical.
	 *     Make sure your implementation is quick and resource friendly.
	 * \li There is no need to call the base class implementation,
	 *     because it simply returns empty ("") strings.
	 * \li The \c result buffer (pointer) is owned by Nlview;
	 *     do not free or reallocate it!
	 * \li To avoid a buffer overflow, \b never copy more than \c len
	 *     characters (+1 for the terminating \c NULL) into the \c result
	 *     buffer.
	 *
	 * \par Example:
	 * \li Nlview wants to display label \c "SYS/ALU/ADD32"
	 *     originating from an instance's "@name" attribute (or default).
	 * \li The space reserved for the label is only 10 characters
         *      (e.g. due to Nlview configure property
         *      <a href="../nlviewCore.html#PROP_instattrmax">instattrmax</a>
         *      set to 10).
	 * \li The label's length is 13 (>10), so it must be elided.
	 * \li Nlview will call
	 *     \code{.cpp}
		NlvQWidget::getCustomElidedText(result, "SYS/ALU/ADD32", 13, 10,
					NlviewList::O_Inst, "@name", false);
	       \endcode
	 * \li Your implementation returns a NULL-terminated string with up to
	 *     10 characters:
	 *     \code{.cpp}
		if (otype == NlviewList::O_Inst && strcmp(aname, "@name")==0) {
		    // path name: elide intermediate hierarchies
		    //            (details omitted)
		    strcpy(result, "./../ADD32");
		} else {
		    // no path name: elide differently, e.g. trailing characters
		    strncpy(result, text, maxlen);
		    result[maxlen] = 0;
		    if (maxlen > 1) result[maxlen-1] = '.';
		    if (maxlen > 2) result[maxlen-2] = '.';
		}
	       \endcode
	 *
	 * \param[out] result The elided text.
	 * \param[in]  text   The given (un-elided) text.
	 * \param[in]  len    The length of the given text and the size
	 *		      of the result buffer (number of allocated
	 *		      characters excluding the terminating \c NULL).
	 * \param[in]  maxlen The maximum number of characters that will fit
	 * 		      the available drawing space.
	 * \param[in]  otype  Additional information: the type of object
	 *		      the text is associated with.
	 * \param[in]  aname  Additional information: the attribute's name.
	 * \param[in]  plan   Additional information: Reserved for future use.
	 *
	 * \sa #setCustomEliding,
	 *     <a href="../nlviewCore.html#PROP_elidetext">elidetext</a>,
	 *     <a href="../nlviewCore.html#ATTRIBUTES_display">
	 *     Display Attributes</a>
	 */
	virtual void getCustomElidedText(char* result,
			const char* text, int len, int maxlen,
			enum NlviewList::OType otype,
			const char* aname, bool plan);
 public:
	/*!
	 * \brief
	 *   Enable calls to #getCustomElidedText
	 *
	 * This function allows to set the text
	 * <a href="../nlviewCore.html#PROP_elidetext">eliding</a> policy
	 * to Nlview's built-in policy (\c false) or to a custom policy
	 * (\c true) as defined by a reimplementation of #getCustomElidedText.
	 *
	 * \param[in]  custom If \c true, then Nlview will query elided text
	 *                    thru calls to #getCustomElidedText function.
	 *                    If \c false, then Nlview will perform text
	 *                    eliding according to its built-in policy.
	 *
	 * \sa #getCustomElidedText, #getCustomEliding,
	 *     <a href="../nlviewCore.html#PROP_elidetext">elidetext</a>
	 */
	void setCustomEliding(bool custom);

	/*!
	 * \brief
	 *   Returns whether custom text eliding is currently enabled or not.
	 *
	 * Text eliding can be toggled between built-in and custom eliding
	 * with a call to #setCustomEliding member function.
	 *
	 * \returns \c true, if Nlview should call #getCustomElidedText
	 *          function to abbreviate labels, \c false if Nlview is
	 *          abbreviating (eliding) labels with its own built-in rules.
	 *
	 * \sa #setCustomEliding, #getCustomElidedText,
	 *     <a href="../nlviewCore.html#PROP_elidetext">elidetext</a>
	 */
	bool getCustomEliding() const;

	/*!
	 * \brief
	 *   Reimplemented from QWidget class to tell our Qt renderer about
	 *   the new font to be used when querying font metrics for schematic
	 *   labels.
	 *
	 * The \c font argument is the new font to be used for both displaying
	 * and generating the schematic. If called after schematic generation,
	 * you may have to regenerate the schematic again - based on the new
	 * font metrics for this font.
	 *
	 * \param[in] font the new font to be used for Nlview from now on.
	 */
	virtual void setFont(const QFont& font);

 private slots:
	// ******************************************************************
	// The following private slots are for internal use only:
	// ******************************************************************

 	/*!
	 * When setting a new horizontal scrollbar (see #setHScrollBar )
	 * we connect this slot to it to be informed about value updates.
	 *
	 * \param[in] The current scrollbar value.
	 */
	void hSBarValueChanged(int);

 	/*!
	 * When setting a new vertical scrollbar (see #setVScrollBar )
	 * we connect this slot to it to be informed about value updates.
	 *
	 * \param[in] The current scrollbar value.
	 */
	void vSBarValueChanged(int);

 	/*!
	 * \internal
	 * \brief
	 *   This method is a slot that will be connected to
	 *   signal NlvQWidget::imageMapChanged.
	 *
	 * Whenever the list of registered images changes, then this
	 * slot will be called to update the SCREEN renderer.
	 * For other short-living Qt renderers (like e.g. for printing
	 * and minimap) we explicitly must call NlvRQt_setImageMap.
	 * Not to be called from user code.
	 */
	void imageMapChangedSlot();

 	/*!
	 * \internal
	 * used by single-shot timer in member function #waitInMainloop.
	 */
	void timeout() { elapsed = true; };


 private:
	// ******************************************************************
	// The following functions are for wrapper-internal use only;
	// ******************************************************************

	/*!
	 * \internal
	 * Returns \c true, if the scrollbars should currently not be updated
	 * with new values (causing other functions to be called). This
	 * function is used internally be the wrapper and needs not to 
	 * be called explicitly.
	 *
	 * \returns \c true, if scrollbar updates should be prevented.
	 */
	bool getDontUpdateSBValues() const { return dontUpdateSBValues; };

	/*!
	 * \internal
	 * Disables or enables the update-behavior of the scrollbars
	 * associated with this widget. This must be done to prevent infinite
	 * recursive loops.
	 * This function is used internally be the wrapper and needs not to 
	 * be called explicitly.
	 *
	 * \param[in] stop \c true, if scrollbar updates should be prevented.
	 */
	void setDontUpdateSBValues(bool stop) { dontUpdateSBValues = stop; };

	/*!
	 * \internal
	 * Here we check for lost redraw events in "failed_draw" due to
	 * non-reentrant painter code.
	 */
	static void checkFailedDraw();

	/*!
	 * \internal
	 */
	void deleteQPixmap(QPixmap* pixmap);

	/*!
	 * \internal
	 */
	QPixmap* createQPixmap(int w, int h);

	/*!
	 * \internal
	 * \param[in] newx new Nlview x coord
	 * \param[in] newy new Nlview y coord
	 */
	void setOrigin(int newx, int newy);

	/*!
	 * \internal
	 * This scrolls the Nlview (core) widget over the visible area
	 * (this widget).
	 * (newx, newy) will match (0,0) which is the upper left corner
	 * of this widget.
	 *
	 * \param[in] newx  New X coordinate (core coordinate)
	 *                  to match the left side of the widget.
	 * \param[in] newy  New Y coordinate (core coordinate)
	 *                  to match the upper side of the widget.
	 */
	void scrollTo(int newX, int newY);

	/*!
	 * \internal
	 * Shift the off-screen buffer's contents and enlarge damage area.
	 *
	 * \param[in] dx  delta-X value: number of pixels to shift horizontally
	 * \param[in] dy  delta-Y value: number of pixels to shift vertically
	 */
	void copy_pixmap(int dx, int dy);

	/*!
	 * \internal
	 * Update the scrollbar ranges to reflect the
	 * scrollable area as previously defined by the NlvCore.
	 * But make sure that the scrollbar values still reflect
	 * the 'scrollpos' (which is a fix point in this operation).
	 */
	void updateScrollbars();

	/*!
	 * \internal
	 * This function is called from #paintEvent handler.
	 * It limits the given area to the screen area.
	 * The given draw-window rectangle is defined in zoomed, unscrolled
	 * Nlview coords.
	 *
	 * \param[in,out] x1  left edge of draw window
	 * \param[in,out] y1  top edge of draw window
	 * \param[in,out] x2  right edge of draw window
	 * \param[in,out] y2  bottom edge of draw window
	 * \param[in] w       border width
	 */
	void limit(int& x1, int& y1, int& x2, int& y2, int w = 0);

	/*!
	 * \internal
	 * Invalidate complete double buffer pixmap.
	 */
	void pixmapDamageAll();

	/*!
	 * \internal
	 * Invalidate parts of double buffer pixmap.
	 * The given damage-window rectangle is defined in zoomed, unscrolled
	 * Nlview coords.
	 *
	 * \param[in] x1  left edge of damage window
	 * \param[in] y1  top edge of damage window
	 * \param[in] x2  right edge of damage window
	 * \param[in] y2  bottom edge of damage window
	 */
        void pixmapDamage(int x1,int y1, int x2,int y2);

	/*!
	 * \internal
	 * This function is responsible for emitting a Qt Signal called
	 * #pageNotify.
	 * This helper member-function had to be implemented because
	 * Qt can only emit signals from within a "Qt"  class.
	 */
	void emitPageNotify(unsigned,int,int);

	/*!
	 * \internal
	 * This function is responsible for emitting a Qt Signal called
	 * #selectionNotify.
	 * This helper member-function had to be implemented because
	 * Qt can only emit signals from within a "Qt"  class.
	 */
	void emitSelectionNotify(unsigned);

	/*!
	 * \internal
	 * This function is responsible for emitting a Qt Signal called
	 * #hierarchyDown.
	 * This helper member-function had to be implemented because
	 * Qt can only emit signals from within a "Qt"  class.
	 *
	 * \param[in] instname  Name of the instance to go down to.
	 */
	void emitHierarchyDown(const char* instname);

	/*!
	 * \internal
	 * This function is responsible for emitting a Qt Signal called
	 * #hierarchyUp.
	 * This helper member-function had to be implemented because
	 * Qt can only emit signals from within a "Qt"  class.
	 */
	void emitHierarchyUp();

	/*!
	 * \internal
	 * This function is responsible for emitting a Qt Signal called
	 * #bindCallback.
	 * This helper member-function had to be implemented because
	 * Qt can only emit signals from within a "Qt"  class.
	 *
	 * \param[in] binding_type  the event which caused this signal
	 * \param[in] down_x  the x position when the mouse button was clicked
	 * \param[in] down_y  the y position when the mouse button was clicked
	 * \param[in] up_x  the x position when the mouse button was released
	 * \param[in] up_y  the y position when the mouse button was released
	 * \param[in] wdelta  the mouse wheel delta in 1/8th of a degree
	 * \param[in] oid   the object_id
	 */
	void emitBindcallback(const char* binding_type,
			      int down_x, int down_y, int up_x, int up_y,
			      int wdelta, const char* oid);

	/*!
	 * \internal
	 * This function is responsible for emitting a Qt Signal called
	 * #messageOutput.
	 * This helper member-function had to be implemented because
	 * Qt can only emit signals from within a "Qt"  class.
	 *
	 * \param[in] id   The kind of message, for example "ERR" or "WAR"
	 * \param[in] txt  The detailed message
	 */
	void emitMessageOutput(const char* id, const char* txt);

	/*!
	 * \internal
	 * This function is responsible for emitting a Qt Signal called
	 * #progressNotify.
	 * This helper member-function had to be implemented because
	 * Qt can only emit signals from within a "Qt"  class.
	 *
	 * \param[in] cnt A "heartbeat"; number of #progressNotify signals
	 *                emitted since command has been started.
	 *                The special number \c -1 denotes the last call.
	 * \param[in] percent The percentage of progress from
	 *                    0.0 to 100.0 (a rough guessing)
	 * \param[in] msg Internal message about what is currently being done.
	 */
	void emitProgressNotify(int cnt, float percent, const char* msg);

	/*!
	 * \internal
	 * This function is a debugging function to insert artificial
	 * delays between incremental updates (to visualize the updates
	 * for humans). Used by 'dbg -wait_before_incr <ms>'.
	 * Paint and timer events are processed while waiting.
	 *
	 * \param[in] ms time in milliseconds to delay program execution
	 */
	void waitInMainloop(int ms);

	/*!
	 * \internal
	 * used by member function #waitInMainloop
	 */
	bool elapsed;

	/*!
	 * \internal
	 * Local copy of property shownetattr
	 */
	int local_shownetattr;

	/*!
	 * \internal
	 * Local copy of property showgrid
	 */
	int local_showgrid;

	/*!
	 * \internal
	 * Flag that handles permission to update scrollbar values.
	 * Use #setDontUpdateSBValues and #dontUpdateSBValues
	 * to modify and receive the current update-permission.
	 */
	bool dontUpdateSBValues;
	
	/*!
	 * \internal
	 * Upper left corner of damaged area, in integer widget coordinates
	 * (zoomed, unscrolled Nlview coords).
	 * Border pixels are included.  Only valid if DAMAGED flag is set.
	 */
	int damageX1, damageY1;

	/*!
	 * \internal
	 * Lower right corner of damaged area, in integer widget coordinates.
	 * Border pixels are *not* included.
	 * 
	 */
	int damageX2, damageY2;

	/*!
	 * \internal
	 * Holds the name of a logfile. Initialized with an empty string.
	 * Use function #setLogfile and #getLogfile to modify/query
	 * Nlview's logfile.
	 */
 	QString logFile;
     
	/*!
	 * \internal
	 * \brief
	 *   The horizontal scrollbar attached to this widget.
	 *
	 * \sa #setHScrollBar, #getHScrollBar #setVScrollBar, #getVScrollBar
	 */
	QScrollBar* hScroll;

	/*!
	 * \internal
	 * \brief The vertical scrollbar attached to this widget.
	 *
	 * \sa #setHScrollBar, #getHScrollBar #setVScrollBar, #getVScrollBar
	 */
	QScrollBar* vScroll;

	/*! 
	 * This extended core structure stores the C-Level callback
	 * function vector. Every function has to be implemented by
	 * the wrapper. Additionally some wrapper specific "private" data
	 * is used in that struct.
	 */
	struct WidgetImpl* widi;

	/*!
	 * \internal
	 * Remember the old cursor when setting a new one;
	 * so we can restore it afterwards.
	 */
	QCursor oldCursor;

	/*!
	 * \internal
	 * Tells if we should enable perfect font scaling mode (experimental).
	 */
	bool perfectfs;

	/*!
	 * \internal
	 * Container of registered image objects.
	 */
	ImageMap* imageMap;

	/*!
	 * \internal
	 * Set of registered minimaps.
	 * We need to unlink them in the #destructor,
	 * if they are still linked at that time.
	 */
#if QT_VERSION >= 0x040000
	QList<NlviewMinimap*>*    minimaps;
#elif QT_VERSION >= 300		/* QList is not generally available in Qt 3 */
	QPtrList<NlviewMinimap>*  minimaps;
#else
	QList<NlviewMinimap>*     minimaps;
#endif


 public:
	/*!
	 * \brief
	 *   This method connects the given NlviewMinimap object to this
	 *   Nlview widget.
	 *
	 * After the link between the given minimap and this Nlview window
	 * has been established, the minimap will automatically be synchronized
	 * with the connected Nlview window whenever its viewport or
	 * netlist (contents) changes.
	 * Call #unlinkMinimap to break that connection again -
	 * or let the destructors do that automatically.
	 * You can link several NlviewMinimap objects to a single Nlview
	 * window (n:1 relation).
	 *
	 * \param[in] minimap The NlviewMinimap object to connect to.
	 * \sa #unlinkMinimap, NlviewMinimap
	 */
	void   linkMinimap(NlviewMinimap* minimap);

	/*!
	 * \brief
	 *   This method disconnects the given NlviewMinimap object from this
	 *   Nlview widget.
	 *
	 * Nlview will stop synchronization of its viewport and contents
	 * with the given minimap, which had been linked with this Nlview
	 * widget by a prior call to #linkMinimap.
	 *
	 * \param[in] minimap The NlviewMinimap object to disconnect from.
	 * \sa #linkMinimap, NlviewMinimap
	 */
	void unlinkMinimap(NlviewMinimap* minimap);

#ifndef SKIP_DOCUMENTATION
 private:
	/*!
	 * \internal
	 * This function is responsible for emitting a Qt Signal called
	 * #mapChanged.
	 * This helper member-function had to be implemented because
	 * Qt can only emit signals from within a "Qt"  class.
	 *
	 * \param[in] scrollr A rectangle specifying the new map dimension in
	 *                    WIDGET coordinates.
	 */
	void emitMapChanged(const QRect& scrollr);

	friend class NlviewMinimap;

	// ******************************************************************
	// Internal use only.
	// 'struct WidgetImpl' needs to access some of our private
	// members in the following extern "C" functions. Declaring them
	// "friend" avoids having certain private members 'public'.
	// ******************************************************************
	friend void vDefineScrollregion(  NLV*,WBox);
	friend void vUndefineScrollregion(NLV*);
	friend void vDamageAll(           NLV*);
	friend void vDamage(              NLV*,WBox);
	friend void vResetAll(            NLV*);
	friend void vCenterAt(            NLV*,int,int);
	friend void vPageNotify(          NLV*,unsigned,int,int);
	friend void vSelectionNotify(     NLV*,unsigned);
	friend void vHierarchyGodown(     NLV*,const char*);
	friend void vHierarchyGoup(       NLV*);
	friend void vBindcallback(        NLV*,const char*,int,int,int,int,
							    int,const char*);
	friend void vMessageOutput(       NLV*,const char*,const char*);
	friend void vProgressNotify(      NLV*,int,float,const char*);
	friend void vPropertyNotify(      NLV*,const char*,const char*);
	friend void vWaitInMainloop(      NLV*,int);
    	friend void vGetElidedText(       NLV*,char*,const char*,int,int,
						int,const char*,int);
	friend void vBusyCursor(          NLV*);
	friend void vStdCursor(           NLV*);
	friend void vStartPan(            NLV*,int,int);
	friend void vPanTo(               NLV*,int,int);
	friend void vStopPan(             NLV*,int,int);
	friend void vInvalidate(          NLV*,WBox);
#undef NLV
#endif // SKIP_DOCUMENTATION


 public:
	// ******************************************************************
	// The following Q_PROPERTY macros are expanded by "moc" to
	// provide Qt-style property definitions for this class...
	// They're accompanied by their corresponding get/set access functions
	// ******************************************************************

#ifndef SKIP_DOCUMENTATION
	/*!
	 * The following access/modifier functions handle the corresponding
	 * Nlview properties. You can read more about Nlview's properties
	 * in the Nlview core API documentation: 
	 * doc/nlviewCore.html#COMMAND_property
	 */
	// INSERT PROPDEF HERE
Q_PROPERTY(int Actionpick READ getActionpick WRITE setActionpick)
Q_PROPERTY(bool Allpageconn READ getAllpageconn WRITE setAllpageconn)
Q_PROPERTY(int Analoglayout READ getAnaloglayout WRITE setAnaloglayout)
Q_PROPERTY(QColor Attrcolor READ getAttrcolor WRITE setAttrcolor)
Q_PROPERTY(int Attrfontsize READ getAttrfontsize WRITE setAttrfontsize)
Q_PROPERTY(int Autobundle READ getAutobundle WRITE setAutobundle)
Q_PROPERTY(int Autohidestyle READ getAutohidestyle WRITE setAutohidestyle)
Q_PROPERTY(QColor Backgroundcolor READ getBackgroundcolor WRITE setBackgroundcolor)
Q_PROPERTY(QColor Boxcolor0 READ getBoxcolor0 WRITE setBoxcolor0)
Q_PROPERTY(QColor Boxcolor1 READ getBoxcolor1 WRITE setBoxcolor1)
Q_PROPERTY(QColor Boxcolor2 READ getBoxcolor2 WRITE setBoxcolor2)
Q_PROPERTY(QColor Boxcolor3 READ getBoxcolor3 WRITE setBoxcolor3)
Q_PROPERTY(int Boxfontsize READ getBoxfontsize WRITE setBoxfontsize)
Q_PROPERTY(int Boxhierpins READ getBoxhierpins WRITE setBoxhierpins)
Q_PROPERTY(QColor Boxinstcolor READ getBoxinstcolor WRITE setBoxinstcolor)
Q_PROPERTY(int Boxinstfontsize READ getBoxinstfontsize WRITE setBoxinstfontsize)
Q_PROPERTY(int Boxmaxwidth READ getBoxmaxwidth WRITE setBoxmaxwidth)
Q_PROPERTY(int Boxmingap READ getBoxmingap WRITE setBoxmingap)
Q_PROPERTY(int Boxminheight READ getBoxminheight WRITE setBoxminheight)
Q_PROPERTY(int Boxminwidth READ getBoxminwidth WRITE setBoxminwidth)
Q_PROPERTY(QColor Boxpincolor READ getBoxpincolor WRITE setBoxpincolor)
Q_PROPERTY(int Boxpinfontsize READ getBoxpinfontsize WRITE setBoxpinfontsize)
Q_PROPERTY(int Boxpingrid READ getBoxpingrid WRITE setBoxpingrid)
Q_PROPERTY(int Boxpinsquare READ getBoxpinsquare WRITE setBoxpinsquare)
Q_PROPERTY(QColor Buscolor READ getBuscolor WRITE setBuscolor)
Q_PROPERTY(int Buswidthlimit READ getBuswidthlimit WRITE setBuswidthlimit)
Q_PROPERTY(int Buswireexcess READ getBuswireexcess WRITE setBuswireexcess)
Q_PROPERTY(int Closeenough READ getCloseenough WRITE setCloseenough)
Q_PROPERTY(int Createnetattrdsp READ getCreatenetattrdsp WRITE setCreatenetattrdsp)
Q_PROPERTY(int Createvconn READ getCreatevconn WRITE setCreatevconn)
Q_PROPERTY(int Createvconn2 READ getCreatevconn2 WRITE setCreatevconn2)
Q_PROPERTY(int Decorate READ getDecorate WRITE setDecorate)
Q_PROPERTY(int Elidetext READ getElidetext WRITE setElidetext)
Q_PROPERTY(bool Fastpanning READ getFastpanning WRITE setFastpanning)
Q_PROPERTY(QColor Fillcolor1 READ getFillcolor1 WRITE setFillcolor1)
Q_PROPERTY(QColor Fillcolor2 READ getFillcolor2 WRITE setFillcolor2)
Q_PROPERTY(QColor Fillcolor3 READ getFillcolor3 WRITE setFillcolor3)
Q_PROPERTY(bool Fillednegpins READ getFillednegpins WRITE setFillednegpins)
Q_PROPERTY(int Fitpage READ getFitpage WRITE setFitpage)
Q_PROPERTY(bool Flyingsegments READ getFlyingsegments WRITE setFlyingsegments)
Q_PROPERTY(QColor Framecolor READ getFramecolor WRITE setFramecolor)
Q_PROPERTY(int Framefontsize READ getFramefontsize WRITE setFramefontsize)
Q_PROPERTY(int Gatecellname READ getGatecellname WRITE setGatecellname)
Q_PROPERTY(int Gatepinname READ getGatepinname WRITE setGatepinname)
Q_PROPERTY(int Geitarget READ getGeitarget WRITE setGeitarget)
Q_PROPERTY(int Greymode READ getGreymode WRITE setGreymode)
Q_PROPERTY(int Grid READ getGrid WRITE setGrid)
Q_PROPERTY(QColor Gridcolor READ getGridcolor WRITE setGridcolor)
Q_PROPERTY(bool Groupregionpins READ getGroupregionpins WRITE setGroupregionpins)
Q_PROPERTY(bool Hiattrvalue READ getHiattrvalue WRITE setHiattrvalue)
Q_PROPERTY(bool Horizontallabels READ getHorizontallabels WRITE setHorizontallabels)
Q_PROPERTY(int Instattrmax READ getInstattrmax WRITE setInstattrmax)
Q_PROPERTY(int Instdrag READ getInstdrag WRITE setInstdrag)
Q_PROPERTY(int Instorder READ getInstorder WRITE setInstorder)
Q_PROPERTY(int Ioorder READ getIoorder WRITE setIoorder)
Q_PROPERTY(int Latchfblevel READ getLatchfblevel WRITE setLatchfblevel)
Q_PROPERTY(bool Mapbool2pla READ getMapbool2pla WRITE setMapbool2pla)
Q_PROPERTY(int Markgap READ getMarkgap WRITE setMarkgap)
Q_PROPERTY(int Marksize READ getMarksize WRITE setMarksize)
Q_PROPERTY(int Matrixalignment READ getMatrixalignment WRITE setMatrixalignment)
Q_PROPERTY(double Maxfontsize READ getMaxfontsize WRITE setMaxfontsize)
Q_PROPERTY(double Maxfontsizecg READ getMaxfontsizecg WRITE setMaxfontsizecg)
Q_PROPERTY(double Maxzoom READ getMaxzoom WRITE setMaxzoom)
Q_PROPERTY(bool Mergepgnets READ getMergepgnets WRITE setMergepgnets)
Q_PROPERTY(int Mergetracks READ getMergetracks WRITE setMergetracks)
Q_PROPERTY(int Minchannelwidth READ getMinchannelwidth WRITE setMinchannelwidth)
Q_PROPERTY(int Minlevelwidth READ getMinlevelwidth WRITE setMinlevelwidth)
Q_PROPERTY(int Netattrmax READ getNetattrmax WRITE setNetattrmax)
Q_PROPERTY(int Netautohide READ getNetautohide WRITE setNetautohide)
Q_PROPERTY(QColor Netcolor READ getNetcolor WRITE setNetcolor)
Q_PROPERTY(int Netfontsize READ getNetfontsize WRITE setNetfontsize)
Q_PROPERTY(int Nethidedetails READ getNethidedetails WRITE setNethidedetails)
Q_PROPERTY(int Netstubminlen READ getNetstubminlen WRITE setNetstubminlen)
Q_PROPERTY(bool Nohierattrs READ getNohierattrs WRITE setNohierattrs)
Q_PROPERTY(bool Nohiernegpins READ getNohiernegpins WRITE setNohiernegpins)
Q_PROPERTY(QColor Objectgrey READ getObjectgrey WRITE setObjectgrey)
Q_PROPERTY(QColor Objecthighlight0 READ getObjecthighlight0 WRITE setObjecthighlight0)
Q_PROPERTY(QColor Objecthighlight1 READ getObjecthighlight1 WRITE setObjecthighlight1)
Q_PROPERTY(QColor Objecthighlight2 READ getObjecthighlight2 WRITE setObjecthighlight2)
Q_PROPERTY(QColor Objecthighlight3 READ getObjecthighlight3 WRITE setObjecthighlight3)
Q_PROPERTY(QColor Objecthighlight4 READ getObjecthighlight4 WRITE setObjecthighlight4)
Q_PROPERTY(QColor Objecthighlight5 READ getObjecthighlight5 WRITE setObjecthighlight5)
Q_PROPERTY(QColor Objecthighlight6 READ getObjecthighlight6 WRITE setObjecthighlight6)
Q_PROPERTY(QColor Objecthighlight7 READ getObjecthighlight7 WRITE setObjecthighlight7)
Q_PROPERTY(QColor Objecthighlight8 READ getObjecthighlight8 WRITE setObjecthighlight8)
Q_PROPERTY(QColor Objecthighlight9 READ getObjecthighlight9 WRITE setObjecthighlight9)
Q_PROPERTY(QColor Objecthighlight10 READ getObjecthighlight10 WRITE setObjecthighlight10)
Q_PROPERTY(QColor Objecthighlight11 READ getObjecthighlight11 WRITE setObjecthighlight11)
Q_PROPERTY(QColor Objecthighlight12 READ getObjecthighlight12 WRITE setObjecthighlight12)
Q_PROPERTY(QColor Objecthighlight13 READ getObjecthighlight13 WRITE setObjecthighlight13)
Q_PROPERTY(QColor Objecthighlight14 READ getObjecthighlight14 WRITE setObjecthighlight14)
Q_PROPERTY(QColor Objecthighlight15 READ getObjecthighlight15 WRITE setObjecthighlight15)
Q_PROPERTY(QColor Objecthighlight16 READ getObjecthighlight16 WRITE setObjecthighlight16)
Q_PROPERTY(QColor Objecthighlight17 READ getObjecthighlight17 WRITE setObjecthighlight17)
Q_PROPERTY(QColor Objecthighlight18 READ getObjecthighlight18 WRITE setObjecthighlight18)
Q_PROPERTY(QColor Objecthighlight19 READ getObjecthighlight19 WRITE setObjecthighlight19)
Q_PROPERTY(bool Ongrid READ getOngrid WRITE setOngrid)
Q_PROPERTY(int Outfblevel READ getOutfblevel WRITE setOutfblevel)
Q_PROPERTY(QColor Overlapcolor READ getOverlapcolor WRITE setOverlapcolor)
Q_PROPERTY(QColor Pbuscolor READ getPbuscolor WRITE setPbuscolor)
Q_PROPERTY(QColor Pbusnamecolor READ getPbusnamecolor WRITE setPbusnamecolor)
Q_PROPERTY(int Pinattrmax READ getPinattrmax WRITE setPinattrmax)
Q_PROPERTY(int Pinautohide READ getPinautohide WRITE setPinautohide)
Q_PROPERTY(int Pinorder READ getPinorder WRITE setPinorder)
Q_PROPERTY(bool Pinpermute READ getPinpermute WRITE setPinpermute)
Q_PROPERTY(int Portattrmax READ getPortattrmax WRITE setPortattrmax)
Q_PROPERTY(QColor Portcolor READ getPortcolor WRITE setPortcolor)
Q_PROPERTY(QColor Portnamecolor READ getPortnamecolor WRITE setPortnamecolor)
Q_PROPERTY(int Reducejogs READ getReducejogs WRITE setReducejogs)
Q_PROPERTY(int Ripattrmax READ getRipattrmax WRITE setRipattrmax)
Q_PROPERTY(int Ripindexfontsize READ getRipindexfontsize WRITE setRipindexfontsize)
Q_PROPERTY(QColor Rippercolor READ getRippercolor WRITE setRippercolor)
Q_PROPERTY(int Rippershape READ getRippershape WRITE setRippershape)
Q_PROPERTY(QColor Rubberbandcolor READ getRubberbandcolor WRITE setRubberbandcolor)
Q_PROPERTY(int Rubberbandfontsize READ getRubberbandfontsize WRITE setRubberbandfontsize)
Q_PROPERTY(int Selectattr READ getSelectattr WRITE setSelectattr)
Q_PROPERTY(int Selectionappearance READ getSelectionappearance WRITE setSelectionappearance)
Q_PROPERTY(QColor Selectioncolor READ getSelectioncolor WRITE setSelectioncolor)
Q_PROPERTY(int Shadowstyle READ getShadowstyle WRITE setShadowstyle)
Q_PROPERTY(double Sheetheight READ getSheetheight WRITE setSheetheight)
Q_PROPERTY(double Sheetwidth READ getSheetwidth WRITE setSheetwidth)
Q_PROPERTY(bool Shortobjid READ getShortobjid WRITE setShortobjid)
Q_PROPERTY(int Showattribute READ getShowattribute WRITE setShowattribute)
Q_PROPERTY(bool Showcellname READ getShowcellname WRITE setShowcellname)
Q_PROPERTY(int Showframe READ getShowframe WRITE setShowframe)
Q_PROPERTY(int Showgrid READ getShowgrid WRITE setShowgrid)
Q_PROPERTY(int Showhierpinname READ getShowhierpinname WRITE setShowhierpinname)
Q_PROPERTY(bool Showinstname READ getShowinstname WRITE setShowinstname)
Q_PROPERTY(bool Showinvisibles READ getShowinvisibles WRITE setShowinvisibles)
Q_PROPERTY(int Showlevels READ getShowlevels WRITE setShowlevels)
Q_PROPERTY(int Showmarks READ getShowmarks WRITE setShowmarks)
Q_PROPERTY(int Shownetattr READ getShownetattr WRITE setShownetattr)
Q_PROPERTY(bool Shownetname READ getShownetname WRITE setShownetname)
Q_PROPERTY(int Showpagenumbers READ getShowpagenumbers WRITE setShowpagenumbers)
Q_PROPERTY(int Showpgtype READ getShowpgtype WRITE setShowpgtype)
Q_PROPERTY(int Showpinname READ getShowpinname WRITE setShowpinname)
Q_PROPERTY(bool Showportname READ getShowportname WRITE setShowportname)
Q_PROPERTY(int Showripindex READ getShowripindex WRITE setShowripindex)
Q_PROPERTY(int Showval READ getShowval WRITE setShowval)
Q_PROPERTY(int Showvconn READ getShowvconn WRITE setShowvconn)
Q_PROPERTY(int Showvconn2 READ getShowvconn2 WRITE setShowvconn2)
Q_PROPERTY(int Stubattrmax READ getStubattrmax WRITE setStubattrmax)
Q_PROPERTY(int Timelimit READ getTimelimit WRITE setTimelimit)
Q_PROPERTY(int Transistorlayout READ getTransistorlayout WRITE setTransistorlayout)
Q_PROPERTY(int Valattr READ getValattr WRITE setValattr)
Q_PROPERTY(bool Viewnameoptional READ getViewnameoptional WRITE setViewnameoptional)
public:
void setActionpick(const int); int getActionpick() const;
void setAllpageconn(const bool); bool getAllpageconn() const;
void setAnaloglayout(const int); int getAnaloglayout() const;
void setAttrcolor(const QColor&); QColor getAttrcolor() const;
void setAttrfontsize(const int); int getAttrfontsize() const;
void setAutobundle(const int); int getAutobundle() const;
void setAutohidestyle(const int); int getAutohidestyle() const;
void setBackgroundcolor(const QColor&); QColor getBackgroundcolor() const;
void setBoxcolor0(const QColor&); QColor getBoxcolor0() const;
void setBoxcolor1(const QColor&); QColor getBoxcolor1() const;
void setBoxcolor2(const QColor&); QColor getBoxcolor2() const;
void setBoxcolor3(const QColor&); QColor getBoxcolor3() const;
void setBoxfontsize(const int); int getBoxfontsize() const;
void setBoxhierpins(const int); int getBoxhierpins() const;
void setBoxinstcolor(const QColor&); QColor getBoxinstcolor() const;
void setBoxinstfontsize(const int); int getBoxinstfontsize() const;
void setBoxmaxwidth(const int); int getBoxmaxwidth() const;
void setBoxmingap(const int); int getBoxmingap() const;
void setBoxminheight(const int); int getBoxminheight() const;
void setBoxminwidth(const int); int getBoxminwidth() const;
void setBoxpincolor(const QColor&); QColor getBoxpincolor() const;
void setBoxpinfontsize(const int); int getBoxpinfontsize() const;
void setBoxpingrid(const int); int getBoxpingrid() const;
void setBoxpinsquare(const int); int getBoxpinsquare() const;
void setBuscolor(const QColor&); QColor getBuscolor() const;
void setBuswidthlimit(const int); int getBuswidthlimit() const;
void setBuswireexcess(const int); int getBuswireexcess() const;
void setCloseenough(const int); int getCloseenough() const;
void setCreatenetattrdsp(const int); int getCreatenetattrdsp() const;
void setCreatevconn(const int); int getCreatevconn() const;
void setCreatevconn2(const int); int getCreatevconn2() const;
void setDecorate(const int); int getDecorate() const;
void setElidetext(const int); int getElidetext() const;
void setFastpanning(const bool); bool getFastpanning() const;
void setFillcolor1(const QColor&); QColor getFillcolor1() const;
void setFillcolor2(const QColor&); QColor getFillcolor2() const;
void setFillcolor3(const QColor&); QColor getFillcolor3() const;
void setFillednegpins(const bool); bool getFillednegpins() const;
void setFitpage(const int); int getFitpage() const;
void setFlyingsegments(const bool); bool getFlyingsegments() const;
void setFramecolor(const QColor&); QColor getFramecolor() const;
void setFramefontsize(const int); int getFramefontsize() const;
void setGatecellname(const int); int getGatecellname() const;
void setGatepinname(const int); int getGatepinname() const;
void setGeitarget(const int); int getGeitarget() const;
void setGreymode(const int); int getGreymode() const;
void setGrid(const int); int getGrid() const;
void setGridcolor(const QColor&); QColor getGridcolor() const;
void setGroupregionpins(const bool); bool getGroupregionpins() const;
void setHiattrvalue(const bool); bool getHiattrvalue() const;
void setHorizontallabels(const bool); bool getHorizontallabels() const;
void setInstattrmax(const int); int getInstattrmax() const;
void setInstdrag(const int); int getInstdrag() const;
void setInstorder(const int); int getInstorder() const;
void setIoorder(const int); int getIoorder() const;
void setLatchfblevel(const int); int getLatchfblevel() const;
void setMapbool2pla(const bool); bool getMapbool2pla() const;
void setMarkgap(const int); int getMarkgap() const;
void setMarksize(const int); int getMarksize() const;
void setMatrixalignment(const int); int getMatrixalignment() const;
void setMaxfontsize(const double); double getMaxfontsize() const;
void setMaxfontsizecg(const double); double getMaxfontsizecg() const;
void setMaxzoom(const double); double getMaxzoom() const;
void setMergepgnets(const bool); bool getMergepgnets() const;
void setMergetracks(const int); int getMergetracks() const;
void setMinchannelwidth(const int); int getMinchannelwidth() const;
void setMinlevelwidth(const int); int getMinlevelwidth() const;
void setNetattrmax(const int); int getNetattrmax() const;
void setNetautohide(const int); int getNetautohide() const;
void setNetcolor(const QColor&); QColor getNetcolor() const;
void setNetfontsize(const int); int getNetfontsize() const;
void setNethidedetails(const int); int getNethidedetails() const;
void setNetstubminlen(const int); int getNetstubminlen() const;
void setNohierattrs(const bool); bool getNohierattrs() const;
void setNohiernegpins(const bool); bool getNohiernegpins() const;
void setObjectgrey(const QColor&); QColor getObjectgrey() const;
void setObjecthighlight0(const QColor&); QColor getObjecthighlight0() const;
void setObjecthighlight1(const QColor&); QColor getObjecthighlight1() const;
void setObjecthighlight2(const QColor&); QColor getObjecthighlight2() const;
void setObjecthighlight3(const QColor&); QColor getObjecthighlight3() const;
void setObjecthighlight4(const QColor&); QColor getObjecthighlight4() const;
void setObjecthighlight5(const QColor&); QColor getObjecthighlight5() const;
void setObjecthighlight6(const QColor&); QColor getObjecthighlight6() const;
void setObjecthighlight7(const QColor&); QColor getObjecthighlight7() const;
void setObjecthighlight8(const QColor&); QColor getObjecthighlight8() const;
void setObjecthighlight9(const QColor&); QColor getObjecthighlight9() const;
void setObjecthighlight10(const QColor&); QColor getObjecthighlight10() const;
void setObjecthighlight11(const QColor&); QColor getObjecthighlight11() const;
void setObjecthighlight12(const QColor&); QColor getObjecthighlight12() const;
void setObjecthighlight13(const QColor&); QColor getObjecthighlight13() const;
void setObjecthighlight14(const QColor&); QColor getObjecthighlight14() const;
void setObjecthighlight15(const QColor&); QColor getObjecthighlight15() const;
void setObjecthighlight16(const QColor&); QColor getObjecthighlight16() const;
void setObjecthighlight17(const QColor&); QColor getObjecthighlight17() const;
void setObjecthighlight18(const QColor&); QColor getObjecthighlight18() const;
void setObjecthighlight19(const QColor&); QColor getObjecthighlight19() const;
void setOngrid(const bool); bool getOngrid() const;
void setOutfblevel(const int); int getOutfblevel() const;
void setOverlapcolor(const QColor&); QColor getOverlapcolor() const;
void setPbuscolor(const QColor&); QColor getPbuscolor() const;
void setPbusnamecolor(const QColor&); QColor getPbusnamecolor() const;
void setPinattrmax(const int); int getPinattrmax() const;
void setPinautohide(const int); int getPinautohide() const;
void setPinorder(const int); int getPinorder() const;
void setPinpermute(const bool); bool getPinpermute() const;
void setPortattrmax(const int); int getPortattrmax() const;
void setPortcolor(const QColor&); QColor getPortcolor() const;
void setPortnamecolor(const QColor&); QColor getPortnamecolor() const;
void setReducejogs(const int); int getReducejogs() const;
void setRipattrmax(const int); int getRipattrmax() const;
void setRipindexfontsize(const int); int getRipindexfontsize() const;
void setRippercolor(const QColor&); QColor getRippercolor() const;
void setRippershape(const int); int getRippershape() const;
void setRubberbandcolor(const QColor&); QColor getRubberbandcolor() const;
void setRubberbandfontsize(const int); int getRubberbandfontsize() const;
void setSelectattr(const int); int getSelectattr() const;
void setSelectionappearance(const int); int getSelectionappearance() const;
void setSelectioncolor(const QColor&); QColor getSelectioncolor() const;
void setShadowstyle(const int); int getShadowstyle() const;
void setSheetheight(const double); double getSheetheight() const;
void setSheetwidth(const double); double getSheetwidth() const;
void setShortobjid(const bool); bool getShortobjid() const;
void setShowattribute(const int); int getShowattribute() const;
void setShowcellname(const bool); bool getShowcellname() const;
void setShowframe(const int); int getShowframe() const;
void setShowgrid(const int); int getShowgrid() const;
void setShowhierpinname(const int); int getShowhierpinname() const;
void setShowinstname(const bool); bool getShowinstname() const;
void setShowinvisibles(const bool); bool getShowinvisibles() const;
void setShowlevels(const int); int getShowlevels() const;
void setShowmarks(const int); int getShowmarks() const;
void setShownetattr(const int); int getShownetattr() const;
void setShownetname(const bool); bool getShownetname() const;
void setShowpagenumbers(const int); int getShowpagenumbers() const;
void setShowpgtype(const int); int getShowpgtype() const;
void setShowpinname(const int); int getShowpinname() const;
void setShowportname(const bool); bool getShowportname() const;
void setShowripindex(const int); int getShowripindex() const;
void setShowval(const int); int getShowval() const;
void setShowvconn(const int); int getShowvconn() const;
void setShowvconn2(const int); int getShowvconn2() const;
void setStubattrmax(const int); int getStubattrmax() const;
void setTimelimit(const int); int getTimelimit() const;
void setTransistorlayout(const int); int getTransistorlayout() const;
void setValattr(const int); int getValattr() const;
void setViewnameoptional(const bool); bool getViewnameoptional() const;
#endif // SKIP_DOCUMENTATION
};
#undef CLASS_EXPORT
#undef NLV
#endif // WRAPPER_H_
