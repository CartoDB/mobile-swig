/* -----------------------------------------------------------------------------
 * This file is part of SWIG, which is licensed as a whole under version 3 
 * (or any later version) of the GNU General Public License. Some additional
 * terms also apply to certain portions of SWIG. The full details of the SWIG
 * license and copyrights can be found in the LICENSE and COPYRIGHT files
 * included with the SWIG source code as distributed by the SWIG developers
 * and at http://www.swig.org/legal.html.
 *
 * PyDocConverter.h
 *
 * Module to return documentation for nodes formatted for PyDoc
 * ----------------------------------------------------------------------------- */

#ifndef PYDOCCONVERTER_H_
#define PYDOCCONVERTER_H_

#include <list>
#include <string>
#include "swig.h"
#include "DoxygenEntity.h"
#include "DoxygenTranslator.h"

#define DOC_STRING_LENGTH	  64	// characters per line allowed
#define DOC_PARAM_STRING_LENGTH	  30	// characters reserved for param name / type

class PyDocConverter : public DoxygenTranslator {
public:
  PyDocConverter(bool debugTranslator = false, bool debugParser = false);
  String *makeDocumentation(Node *node);

protected:

  size_t m_tableLineLen;
  bool m_prevRowIsTH;
  std::string m_url;

  /*
   * Format a string so it is justified and split over several lines 
   * not exeeding a given length.
   */
  std::string justifyString(std::string unformattedLine, int indent = 0, int maxWidth = DOC_STRING_LENGTH);
  /*
   * Translate every entity in a tree, also manages sections
   * display. Prints title for every group of tags that have
   * a section title associated with them
   */
  std::string translateSubtree(DoxygenEntity & doxygenEntity);
  /*
   * Translate one entity with the appropriate handler, acording
   * to the tagHandlers
   */
  void translateEntity(DoxygenEntity & doxyEntity, std::string &translatedComment);

  /*
   * Typedef for the function that handles one tag
   * arg - some string argument to easily pass it through lookup table
   */
  typedef void (PyDocConverter::*tagHandler)(DoxygenEntity &tag,
      std::string &translatedComment, std::string &arg);

  /*
   * Wrap the command data with the some string
   * arg - string to wrap with, like '_' or '*'
   */
  void handleTagWrap(DoxygenEntity &tag, std::string &translatedComment, std::string &arg);
  /*
   * Just prints new line
   */
  void handleNewLine(DoxygenEntity &tag, std::string &translatedComment, std::string &arg);
  /*
   * Print the name of tag to the output, used for escape-commands
   */
  void handleTagChar(DoxygenEntity &tag, std::string &translatedComment, std::string &arg);
  /*
   * Print only the content and strip original tag
   */
  void handleParagraph(DoxygenEntity &tag, std::string &translatedComment, std::string &arg);
  /*
   * Print only data part of code
   */
  void handlePlainString(DoxygenEntity &tag, std::string &translatedComment, std::string &arg);

  /**
   * Copies verbatim args of the tag to output, used for commands like \f$, ...
   */
  void handleTagVerbatim(DoxygenEntity& tag,
                            std::string& translatedComment,
                            std::string &arg);

  /*
   * Print the if-elseif-else-endif section
   */
  void handleTagIf(DoxygenEntity &tag, std::string &translatedComment, std::string &arg);
  /*
   * Prints the specified message, than the contents of the tag
   * arg - message
   */
  void handleTagMessage(DoxygenEntity &tag, std::string &translatedComment, std::string &arg);
  /*
   * Insert 'Title: ...'
   */
  void handleTagPar(DoxygenEntity &tag, std::string &translatedComment, std::string &arg);
  /*
   * Insert 'Image: ...'
   */
  void handleTagImage(DoxygenEntity &tag, std::string &translatedComment, std::string &arg);
  /*
   * Format nice param description with type information
   */
  void handleTagParam(DoxygenEntity &tag, std::string &translatedComment, std::string &arg);
  /*
   * Writes text for \ref tag. 
   */
  void handleTagRef(DoxygenEntity& tag, std::string& translatedComment, std::string&);
    
  /* Handles HTML tags recognized by Doxygen, like <A ...>, <ul>, <table>, ... */

  void handleDoxyHtmlTag(DoxygenEntity& tag, std::string& translatedComment, std::string &arg);

  /** Does not ouput params of HTML tag, for example in <table border='1'>
   * 'border=1' is not written to output.
   */
  void handleDoxyHtmlTagNoParam(DoxygenEntity& tag,
                                    std::string& translatedComment,
                                    std::string &arg);

  /** TRanslates tag <a href = "url">text</a> to: text ("url"). */
  void handleDoxyHtmlTag_A(DoxygenEntity& tag,
                              std::string& translatedComment,
                              std::string &arg);

  /*
   * Handles HTML tags, which are translated to markdown-like syntax, for example
   * <i>text</i>  -->  _text_. Appends arg for start HTML tag and end HTML tag.
   */
  void handleDoxyHtmlTag2(DoxygenEntity& tag,
                             std::string& translatedComment,
                             std::string &arg);

  /* Handles HTML table, tag <tr> */
  void handleDoxyHtmlTag_tr(DoxygenEntity& tag, std::string& translatedComment,
                               std::string &arg);

  /* Handles HTML table, tag <th> */
  void handleDoxyHtmlTag_th(DoxygenEntity& tag, std::string& translatedComment,
                               std::string &arg);

  /* Handles HTML table, tag <td> */
  void handleDoxyHtmlTag_td(DoxygenEntity& tag, std::string& translatedComment,
                                std::string &arg);

  /* Handles HTML entities recognized by Doxygen, like &lt;, &copy;, ... */
  void handleHtmlEntity(DoxygenEntity&, std::string& translatedComment, std::string &arg);


  /*
   * Utility method to generate a diving line for a documentation string.
   */
  std::string generateDivider();

  /*
   * Simple helper function that calculates correct parameter type
   * of the node stored in 'currentNode'
   * If param with specified name is not found, empty string is returned
   */
  std::string getParamType(std::string name);

private:
  // temporary thing, should be refactored somehow
  Node *currentNode;
  // this contains the handler pointer and one string argument
  static std::map<std::string, std::pair<tagHandler, std::string> > tagHandlers;
  // this cointains the sectins titiles, like 'Arguments:' or 'Notes:', that are printed only once
  static std::map<std::string, std::string> sectionTitles;
  void fillStaticTables();
};

#endif
