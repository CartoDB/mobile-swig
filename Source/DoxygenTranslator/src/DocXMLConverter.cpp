/* -----------------------------------------------------------------------------
* This file is part of SWIG, which is licensed as a whole under version 3
* (or any later version) of the GNU General Public License. Some additional
* terms also apply to certain portions of SWIG. The full details of the SWIG
* license and copyrights can be found in the LICENSE and COPYRIGHT files
* included with the SWIG source code as distributed by the SWIG developers
* and at http://www.swig.org/legal.html.
*
* DocXMLConverter.cpp
* ----------------------------------------------------------------------------- */

#include "DocXMLConverter.h"
#include "DoxygenParser.h"
#include <iostream>
#include <vector>
#include <list>
#include "../../Modules/swigmod.h"
#define APPROX_LINE_LENGTH 64	//characters per line allowed
#define TAB_SIZE 8		//current tab size in spaces
//TODO {@link} {@linkplain} {@docRoot}, and other useful doxy commands that are not a DocXML tag

// define static tables, they are filled in DocXMLConverter's constructor
std::map<std::string, std::pair<DocXMLConverter::tagHandler, std::string> > DocXMLConverter::tagHandlers;

using std::string;
using std::list;
using std::vector;

void DocXMLConverter::fillStaticTables()
{
    if (tagHandlers.size()) // fill only once
        return;

    /*
    * Some translation rules:
    *
    * @ and \ must be escaped for both Java and Python to appear on output: \@, \\,
    *         while Doxygen produces output in both cases.
    * Rule:   @ and \ with space on the right should get to output.
    *
    * :: remains intact, even in class::method(). But you can use class#method also
    *    in C++ comment and it is properly translated to C++ output (changed by doxygen to ::)
    *    and Java output (remains #).
    * Rule: SWIG type system can't be used to convert C::m to C#m, because in Java it is C.m
    *       Use string replacement :: --> # in tag see and links.
    *
    * HTML tags must be translated - remain in Java, to markdown in Python
    *
    * Unknown HTML tags, for example <x> is translated to &lt;x&gt; by doxygen, while
    *     Java src is <x> and therefore invisible on output - browser ignores unknown command.
    *     This is handy in syntax descriptions, for example: more <fileName>.
    *
    * Standlaone < and > need not be translated, they are rendered properly in
    *      all three outputs.
    *
    * ., %, and " need not to be translated
    *
    * entities must be translated - remain in Java, something meaningfull in Python (&lt, ...)
    *
    * - Python
    * - add comments also to auto-generated methods like equals(), delete() in Java,
    *   and methods for std::vector(), ...
    *   Commenting methods of std types is simple - add comment to std_*.i file.
    */

    // these commands insert HTML tags
    tagHandlers["a"] = make_pair(&DocXMLConverter::handleTagHtml, "i");
    tagHandlers["arg"] = make_pair(&DocXMLConverter::handleTagHtml, "li");
    tagHandlers["b"] = make_pair(&DocXMLConverter::handleTagHtml, "b");
    tagHandlers["c"] = make_pair(&DocXMLConverter::handleTagHtml, "code");
    tagHandlers["cite"] = make_pair(&DocXMLConverter::handleTagHtml, "i");
    tagHandlers["e"] = make_pair(&DocXMLConverter::handleTagHtml, "i");
    tagHandlers["em"] = make_pair(&DocXMLConverter::handleTagHtml, "i");
    tagHandlers["li"] = make_pair(&DocXMLConverter::handleTagHtml, "li");
    tagHandlers["p"] = make_pair(&DocXMLConverter::handleTagHtml, "code");
    // these commands insert just a single char, some of them need to be escaped
    tagHandlers["$"] = make_pair(&DocXMLConverter::handleTagChar, "");
    tagHandlers["@"] = make_pair(&DocXMLConverter::handleTagChar, "");
    tagHandlers["\\"] = make_pair(&DocXMLConverter::handleTagChar, "");
    tagHandlers["<"] = make_pair(&DocXMLConverter::handleTagChar, "&lt;");
    tagHandlers[">"] = make_pair(&DocXMLConverter::handleTagChar, "&gt;");
    tagHandlers["&"] = make_pair(&DocXMLConverter::handleTagChar, "&amp;");
    tagHandlers["#"] = make_pair(&DocXMLConverter::handleTagChar, "");
    tagHandlers["%"] = make_pair(&DocXMLConverter::handleTagChar, "");
    tagHandlers["~"] = make_pair(&DocXMLConverter::handleTagChar, "");
    tagHandlers["\""] = make_pair(&DocXMLConverter::handleTagChar, "&quot;");
    tagHandlers["."] = make_pair(&DocXMLConverter::handleTagChar, "");
    tagHandlers["::"] = make_pair(&DocXMLConverter::handleTagChar, "");
    // these commands are stripped out
    tagHandlers["attention"] = make_pair(&DocXMLConverter::handleParagraph, "");
    tagHandlers["anchor"] = make_pair(&DocXMLConverter::handleTagAnchor, "");
    tagHandlers["brief"] = make_pair(&DocXMLConverter::handleParagraph, "");
    tagHandlers["bug"] = make_pair(&DocXMLConverter::handleParagraph, "");
    tagHandlers["date"] = make_pair(&DocXMLConverter::handleParagraph, "");
    tagHandlers["details"] = make_pair(&DocXMLConverter::handleParagraph, "");
    // this command is inserts text accumulated after cmd htmlonly -
    // see DoxygenParser - CMD_HTML_ONLY.
    tagHandlers["htmlonly"] = make_pair(&DocXMLConverter::handleParagraph, "");
    tagHandlers["invariant"] = make_pair(&DocXMLConverter::handleParagraph, "");
    tagHandlers["latexonly"] = make_pair(&DocXMLConverter::handleParagraph, "");
    tagHandlers["manonly"] = make_pair(&DocXMLConverter::handleParagraph, "");
    tagHandlers["partofdescription"] = make_pair(
        &DocXMLConverter::handleParagraph, "");
    tagHandlers["rtfonly"] = make_pair(&DocXMLConverter::handleParagraph, "");
    tagHandlers["short"] = make_pair(&DocXMLConverter::handleParagraph, "");
    tagHandlers["xmlonly"] = make_pair(&DocXMLConverter::handleParagraph, "");
    // these commands are kept as-is, they are supported by DocXML
    tagHandlers["author"] = make_pair(&DocXMLConverter::handleTagSame, "");
    tagHandlers["authors"] = make_pair(&DocXMLConverter::handleTagSame,
        "author");
    tagHandlers["deprecated"] = make_pair(&DocXMLConverter::handleTagSame, "");
    tagHandlers["exception"] = make_pair(&DocXMLConverter::handleTagSame, "");
    tagHandlers["package"] = make_pair(&DocXMLConverter::handleTagSame, "");
    tagHandlers["param"] = make_pair(&DocXMLConverter::handleTagParam, "");
    tagHandlers["tparam"] = make_pair(&DocXMLConverter::handleTagParam, "");
    tagHandlers["ref"] = make_pair(&DocXMLConverter::handleTagRef, "");
    tagHandlers["result"] = make_pair(&DocXMLConverter::handleTagSame, "return");
    tagHandlers["return"] = make_pair(&DocXMLConverter::handleTagReturn, "");
    tagHandlers["returns"] = make_pair(&DocXMLConverter::handleTagSame,
        "return");
    //tagHandlers["see"] = make_pair(&DocXMLConverter::handleTagSame, "");
    //tagHandlers["sa"] = make_pair(&DocXMLConverter::handleTagSame, "see");
    tagHandlers["since"] = make_pair(&DocXMLConverter::handleTagSame, "");
    tagHandlers["throws"] = make_pair(&DocXMLConverter::handleTagSame, "");
    tagHandlers["throw"] = make_pair(&DocXMLConverter::handleTagSame, "throws");
    tagHandlers["version"] = make_pair(&DocXMLConverter::handleTagSame, "");
    // these commands have special handlers
    tagHandlers["code"] = make_pair(&DocXMLConverter::handleTagExtended, "code");
    tagHandlers["cond"] = make_pair(&DocXMLConverter::handleTagMessage,
        "Conditional comment: ");
    tagHandlers["copyright"] = make_pair(&DocXMLConverter::handleTagMessage,
        "Copyright: ");
    tagHandlers["else"] = make_pair(&DocXMLConverter::handleTagIf, "Else: ");
    tagHandlers["elseif"] = make_pair(&DocXMLConverter::handleTagIf,
        "Else if: ");
    tagHandlers["endcond"] = make_pair(&DocXMLConverter::handleTagMessage,
        "End of conditional comment.");
    // space in second arg prevents DocXML to treat '@ example' as command. File name of
    // example is still informative to user.
    tagHandlers["example"] = make_pair(&DocXMLConverter::handleTagSame,
        " example");
    tagHandlers["if"] = make_pair(&DocXMLConverter::handleTagIf, "If: ");
    tagHandlers["ifnot"] = make_pair(&DocXMLConverter::handleTagIf, "If not: ");
    tagHandlers["image"] = make_pair(&DocXMLConverter::handleTagImage, "");
    tagHandlers["link"] = make_pair(&DocXMLConverter::handleTagLink, "");
    tagHandlers["see"] = make_pair(&DocXMLConverter::handleTagSee, "");
    tagHandlers["sa"] = make_pair(&DocXMLConverter::handleTagSee, "");
    tagHandlers["note"] = make_pair(&DocXMLConverter::handleTagMessage,
        "Note: ");
    tagHandlers["overload"] = make_pair(&DocXMLConverter::handleTagMessage,
        "This is an overloaded member function, provided for"
        " convenience. It differs from the above function only in what"
        " argument(s) it accepts.");
    tagHandlers["par"] = make_pair(&DocXMLConverter::handleTagPar, "");
    tagHandlers["remark"] = make_pair(&DocXMLConverter::handleTagMessage,
        "Remarks: ");
    tagHandlers["remarks"] = make_pair(&DocXMLConverter::handleTagMessage,
        "Remarks: ");
    tagHandlers["todo"] = make_pair(&DocXMLConverter::handleTagMessage,
        "TODO: ");
    tagHandlers["verbatim"] = make_pair(&DocXMLConverter::handleTagExtended,
        "literal");

    // \f commands output literal Latex formula, which is still better than nothing.
    tagHandlers["f$"] = make_pair(&DocXMLConverter::handleTagVerbatim, "");
    tagHandlers["f["] = make_pair(&DocXMLConverter::handleTagVerbatim, "");
    tagHandlers["f{"] = make_pair(&DocXMLConverter::handleTagVerbatim, "");

    tagHandlers["warning"] = make_pair(&DocXMLConverter::handleTagMessage,
        "Warning: ");
    // this command just prints it's contents
    // (it is internal command of swig's parser, contains plain text)
    tagHandlers["plainstd::string"] = make_pair(
        &DocXMLConverter::handlePlainString, "");
    tagHandlers["plainstd::endl"] = make_pair(&DocXMLConverter::handleNewLine,
        "");
    tagHandlers["n"] = make_pair(&DocXMLConverter::handleNewLine, "");

    // HTML tags
    tagHandlers["<a"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<a");
    tagHandlers["<b"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<b");
    tagHandlers["<blockquote"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag,
        "<blockquote");
    tagHandlers["<body"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag,
        "<body");
    tagHandlers["<br"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<br");
    tagHandlers["<center"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag,
        "<center");
    tagHandlers["<caption"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag,
        "<caption");
    tagHandlers["<code"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag,
        "<code");
    tagHandlers["<dd"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<dd");
    tagHandlers["<dfn"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<dfn");
    tagHandlers["<div"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<div");
    tagHandlers["<dl"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<dl");
    tagHandlers["<dt"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<dt");
    tagHandlers["<em"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<em");
    tagHandlers["<form"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag,
        "<form");
    tagHandlers["<hr"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<hr");
    tagHandlers["<h1"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<h1");
    tagHandlers["<h2"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<h2");
    tagHandlers["<h3"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<h3");
    tagHandlers["<i"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<i");
    tagHandlers["<input"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag,
        "<input");
    tagHandlers["<img"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<img");
    tagHandlers["<li"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<li");
    tagHandlers["<meta"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag,
        "<meta");
    tagHandlers["<multicol"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag,
        "<multicol");
    tagHandlers["<ol"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<ol");
    tagHandlers["<p"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<p");
    tagHandlers["<pre"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<pre");
    tagHandlers["<small"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag,
        "<small");
    tagHandlers["<span"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag,
        "<span");
    tagHandlers["<strong"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag,
        "<strong");
    tagHandlers["<sub"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<sub");
    tagHandlers["<sup"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<sup");
    tagHandlers["<table"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag,
        "<table");
    tagHandlers["<td"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<td");
    tagHandlers["<th"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<th");
    tagHandlers["<tr"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<tr");
    tagHandlers["<tt"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<tt");
    tagHandlers["<kbd"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<kbd");
    tagHandlers["<ul"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<ul");
    tagHandlers["<var"] = make_pair(&DocXMLConverter::handleDoxyHtmlTag, "<var");

    // HTML entities
    tagHandlers["&copy"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&copy");
    tagHandlers["&trade"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&trade");
    tagHandlers["&reg"] = make_pair(&DocXMLConverter::handleHtmlEntity, "&reg");
    tagHandlers["&lt"] = make_pair(&DocXMLConverter::handleHtmlEntity, "&lt");
    tagHandlers["&gt"] = make_pair(&DocXMLConverter::handleHtmlEntity, "&gt");
    tagHandlers["&amp"] = make_pair(&DocXMLConverter::handleHtmlEntity, "&amp");
    tagHandlers["&apos"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&apos");
    tagHandlers["&quot"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&quot");
    tagHandlers["&lsquo"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&lsquo");
    tagHandlers["&rsquo"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&rsquo");
    tagHandlers["&ldquo"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&ldquo");
    tagHandlers["&rdquo"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&rdquo");
    tagHandlers["&ndash"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&ndash");
    tagHandlers["&mdash"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&mdash");
    tagHandlers["&nbsp"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&nbsp");
    tagHandlers["&times"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&times");
    tagHandlers["&minus"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&minus");
    tagHandlers["&sdot"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&sdot");
    tagHandlers["&sim"] = make_pair(&DocXMLConverter::handleHtmlEntity, "&sim");
    tagHandlers["&le"] = make_pair(&DocXMLConverter::handleHtmlEntity, "&le");
    tagHandlers["&ge"] = make_pair(&DocXMLConverter::handleHtmlEntity, "&ge");
    tagHandlers["&larr"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&larr");
    tagHandlers["&rarr"] = make_pair(&DocXMLConverter::handleHtmlEntity,
        "&rarr");
}

DocXMLConverter::DocXMLConverter(bool debugTranslator, bool debugParser) :
    DoxygenTranslator(debugTranslator, debugParser)
{
    fillStaticTables();
}

/**
* Formats comment lines by inserting '\n *' at to long lines and tabs for
* indent. Currently it is disabled, which means original comment format is
* preserved. Experience shows, that this is usually better than breaking
* lines automatically, especially because original line endings are not removed,
* which results in short lines. To be useful, this function should have much
* better algorithm.
*/
std::string DocXMLConverter::formatCommand(std::string unformattedLine,
    int indent)
{
    std::string formattedLines;
    return unformattedLine;  // currently disabled
    int lastPosition = 0;
    int i = 0;
    int isFirstLine = 1;
    while (i != -1 && i < (int)unformattedLine.length()) {
        lastPosition = i;
        if (isFirstLine) {
            i += APPROX_LINE_LENGTH;
        }
        else {
            i += APPROX_LINE_LENGTH - indent * TAB_SIZE;
        }

        i = unformattedLine.find(" ", i);

        if (i > 0 && i + 1 < (int)unformattedLine.length()) {
            if (!isFirstLine)
                for (int j = 0; j < indent; j++) {
                    formattedLines.append("\t");
                }
            else {
                isFirstLine = 0;
            }
            formattedLines.append(
                unformattedLine.substr(lastPosition, i - lastPosition + 1));
            formattedLines.append("\n *");

        }
    }
    if (lastPosition < (int)unformattedLine.length()) {
        if (!isFirstLine) {
            for (int j = 0; j < indent; j++) {
                formattedLines.append("\t");
            }
        }
        formattedLines.append(
            unformattedLine.substr(lastPosition,
                unformattedLine.length() - lastPosition));
    }

    return formattedLines;
}

/**
* Returns true, if the given parameter exists in the current node
* (for example param is a name of function parameter). If feature
* 'doxygen:nostripparams' is set, then this method always returns
* true - parameters are copied to output regardless of presence in
* function params list.
*/
bool DocXMLConverter::paramExists(std::string param)
{

    if (GetFlag(currentNode, "feature:doxygen:nostripparams")) {
        return true;
    }

    ParmList *plist = CopyParmList(Getattr(currentNode, "parms"));

    for (Parm *p = plist; p;) {

        if (Getattr(p, "name") && Char(Getattr(p, "name")) == param) {
            return true;
        }
        /* doesn't seem to work always: in some cases (especially for 'self' parameters)
        * tmap:in is present, but tmap:in:next is not and so this code skips all the parameters
        */
        //p = Getattr(p, "tmap:in") ? Getattr(p, "tmap:in:next") : nextSibling(p);
        p = nextSibling(p);
    }

    Delete(plist);

    return false;
}

std::string DocXMLConverter::translateSubtree(DoxygenEntity &doxygenEntity)
{
    std::string translatedComment;

    if (doxygenEntity.isLeaf) {
        return translatedComment;
    }

    for (DoxygenEntityListIt p = doxygenEntity.entityList.begin();
    p != doxygenEntity.entityList.end(); p++) {

        translateEntity(*p, translatedComment);
        translateSubtree(*p);
    }

    return translatedComment;
}

/**
* Checks if a handler for the given tag exists, and calls it.
*/
void DocXMLConverter::translateEntity(DoxygenEntity &tag,
    std::string &translatedComment)
{

    std::map<std::string, std::pair<tagHandler, std::string> >::iterator it;
    it = tagHandlers.find(tag.typeOfEntity);

    if (it != tagHandlers.end()) {
        (this->*(it->second.first))(tag, translatedComment, it->second.second);
    }
    else {
        // do NOT print warning, since there are many tags, which are not
        // translatable - many warnings hide important ones
        // addError(WARN_DOXYGEN_COMMAND_ERROR, "Unknown doxygen or HTML tag: " + tag.typeOfEntity);
    }
}


void DocXMLConverter::handleTagAnchor(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string &)
{
    translatedComment += "<a id=\"" + translateSubtree(tag) + "\"></a>";
}


void DocXMLConverter::handleTagHtml(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string &arg)
{
    if (tag.entityList.size()) { // do not include empty tags
        std::string tagData = translateSubtree(tag);
        // wrap the thing, ignoring whitespaces
        size_t wsPos = tagData.find_last_not_of("\n\t ");
        if (wsPos != std::string::npos)
            translatedComment += "<" + arg + ">" + tagData.substr(0, wsPos + 1) + "</"
            + arg + ">" + tagData.substr(wsPos + 1);
        else
            translatedComment += "<" + arg + ">" + translateSubtree(tag) + "</" + arg
            + "> ";
    }
}

void DocXMLConverter::handleDoxyHtmlTag(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string &arg)
{
    std::string htmlTagArgs = tag.data;
    if (htmlTagArgs == "/") {
        // end html tag, for example "</ul>
        translatedComment += "</" + arg.substr(1) + ">";
    }
    else {
        translatedComment += arg + htmlTagArgs + ">";
    }
}

void DocXMLConverter::handleHtmlEntity(DoxygenEntity&,
    std::string& translatedComment,
    std::string &arg)
{
    // html entities can be preserved for Java
    translatedComment += arg + ';';
}

void DocXMLConverter::handleNewLine(DoxygenEntity&,
    std::string& translatedComment,
    std::string&)
{
    // <br> tag is added, because otherwise to much text is joined
    // into same paragraph by DocXML. For example, doxy list:
    // - item one
    // - item two
    // becomes one paragraph with surrounding text without newlines.
    // This way we get to many empty lines in DocXML output, but this
    // is still better than joined lines. Possibility for improvements
    // exists.
    translatedComment += "\n/// ";
}

void DocXMLConverter::handleTagChar(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string &arg)
{
    // escape it if we need to, else just print
    if (arg.size())
        translatedComment += arg;
    else
        translatedComment += tag.typeOfEntity;
}

// handles tags which are the same in Doxygen and DocXML.
void DocXMLConverter::handleTagSame(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string &arg)
{
    if (arg.size())
        tag.typeOfEntity = arg;
    translatedComment += formatCommand(
        std::string("@" + tag.typeOfEntity + " " + translateSubtree(tag)), 2);
}

void DocXMLConverter::handleParagraph(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string&)
{
    if (tag.typeOfEntity == "partofdescription")
        translatedComment += "<summary>\n/// ";
    translatedComment += formatCommand(translateSubtree(tag), 0);
    if (tag.typeOfEntity == "partofdescription")
        translatedComment += "\n/// </summary>";
}

void DocXMLConverter::handlePlainString(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string&)
{
    translatedComment += tag.data;
    // if (tag.data.size() && tag.data[tag.data.size()-1] != ' ')
    // 	translatedComment += " ";
}

void DocXMLConverter::handleTagVerbatim(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string &arg)
{
    translatedComment += arg + " ";
    for (DoxygenEntityListCIt it = tag.entityList.begin();
    it != tag.entityList.end(); it++) {
        translatedComment += it->data;
    }
}

void DocXMLConverter::handleTagExtended(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string &arg)
{
    std::string dummy;
    translatedComment += "{@" + arg + " ";
    handleParagraph(tag, translatedComment, dummy);
    translatedComment += "}";
}

void DocXMLConverter::handleTagIf(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string &arg)
{
    std::string dummy;
    translatedComment += arg;
    if (tag.entityList.size()) {
        translatedComment += tag.entityList.begin()->data;
        tag.entityList.pop_front();
        translatedComment += " {" + translateSubtree(tag) + "}";
    }
}

void DocXMLConverter::handleTagMessage(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string &arg)
{
    std::string dummy;
    translatedComment += formatCommand(arg, 0);
    handleParagraph(tag, translatedComment, dummy);
}

void DocXMLConverter::handleTagImage(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string&)
{
    if (tag.entityList.size() < 2)
        return;

    std::string file;
    std::string title;

    std::list<DoxygenEntity>::iterator it = tag.entityList.begin();
    if (it->data != "html")
        return;

    it++;
    file = it->data;

    it++;
    if (it != tag.entityList.end())
        title = it->data;

    translatedComment += "<img src=" + file;
    if (title.size())
        translatedComment += " alt=" + title;

    // the size indication is supported for Latex only in Doxygen, see manual

    translatedComment += "/>";
}

void DocXMLConverter::handleTagPar(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string&)
{
    std::string dummy;
    translatedComment += "<p";
    if (tag.entityList.size()) {
        translatedComment += " alt=\"" + tag.entityList.begin()->data + "\"";
        translatedComment += ">";
        tag.entityList.pop_front();
        handleParagraph(tag, translatedComment, dummy);
    }
    translatedComment += "</p>";
}


void DocXMLConverter::handleTagParam(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string&)
{
    std::string dummy;

    if (!tag.entityList.size())
        return;
    if (!paramExists(tag.entityList.begin()->data))
        return;

    translatedComment += "<param name=\"" + tag.entityList.begin()->data + "\">";
    tag.entityList.pop_front();
    handleParagraph(tag, translatedComment, dummy);
    translatedComment += "</param>";
}

void DocXMLConverter::handleTagReturn(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string&)
{
    std::string dummy;

    if (!tag.entityList.size())
        return;

    translatedComment += "<returns>";
    handleParagraph(tag, translatedComment, dummy);
    translatedComment += "</returns>";
}

void DocXMLConverter::handleTagRef(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string&)
{
    std::string dummy;
    if (!tag.entityList.size())
        return;

    // we translate to link, although \page is not supported in Java, but 
    // reader at least knows what to look at. Also for \anchor tag on the same
    // page this link works.
    string anchor = tag.entityList.begin()->data;
    tag.entityList.pop_front();
    string anchorText = anchor;
    if (!tag.entityList.empty()) {
        anchorText = tag.entityList.begin()->data;
    }
    translatedComment += "<a href=\"#" + anchor + "\">" + anchorText + "</a>";
}


string DocXMLConverter::convertLink(string linkObject)
{
    if (GetFlag(currentNode, "feature:doxygen:nolinktranslate"))
        return linkObject;
    // find the params in function in linkObject (if any)
    size_t lbracePos = linkObject.find('(', 0);
    size_t rbracePos = linkObject.find(')', 0);
    if (lbracePos == string::npos || rbracePos == string::npos
        || lbracePos >= rbracePos)
        return "";

    string paramsStr = linkObject.substr(lbracePos + 1,
        rbracePos - lbracePos - 1);
    // strip the params, to fill them later
    string additionalObject = linkObject.substr(rbracePos + 1, string::npos);
    linkObject = linkObject.substr(0, lbracePos);

    // find all the params
    vector<string> params;
    size_t lastPos = 0, commaPos = 0;
    while (true) {
        commaPos = paramsStr.find(',', lastPos);
        if (commaPos == string::npos)
            commaPos = paramsStr.size();
        string param = paramsStr.substr(lastPos, commaPos - lastPos);
        // if any param type is empty, we are failed
        if (!param.size())
            return "";
        params.push_back(param);
        lastPos = commaPos + 1;
        if (lastPos >= paramsStr.size())
            break;
    }

    linkObject += "(";
    for (size_t i = 0; i < params.size(); i++) {
        // translate c/c++ type string to swig's type
        // i e 'int **arr[100][10]' -> 'a(100).a(10).p.p.int'
        // also converting arrays to pointers
        string paramStr = params[i];
        String *swigType = NewString("");

        // handle const qualifier
        if (paramStr.find("const") != string::npos)
            SwigType_add_qualifier(swigType, "const");

        // handle pointers, references and arrays
        for (int j = (int)params[i].size() - 1; j >= 0; j--) {
            // skip all the [...] blocks, write 'p.' for every of it
            if (paramStr[j] == ']') {
                while (j >= 0 && paramStr[j] != '[')
                    j--;
                // no closing brace
                if (j < 0)
                    return "";
                SwigType_add_pointer(swigType);
                continue;
            }
            else if (paramStr[j] == '*')
                SwigType_add_pointer(swigType);
            else if (paramStr[j] == '&')
                SwigType_add_reference(swigType);
            else if (isalnum(paramStr[j])) {
                size_t typeNameStart = paramStr.find_last_of(' ', j + 1);
                if (typeNameStart == string::npos)
                    typeNameStart = 0;
                else
                    typeNameStart++;
                Append(swigType,
                    paramStr.substr(typeNameStart, j - typeNameStart + 1).c_str());
                break;
            }
        }

        // make dummy param list, to lookup typemaps for it
        Parm *dummyParam = NewParm(swigType, "", 0);
        Swig_typemap_attach_parms("jstype", dummyParam, NULL);
        Language::instance()->replaceSpecialVariables(0,
            Getattr(dummyParam, "tmap:jstype"), dummyParam);

        //Swig_print(dummyParam, 1);
        linkObject += Char(Getattr(dummyParam, "tmap:jstype"));
        if (i != params.size() - 1)
            linkObject += ",";

        Delete(dummyParam);
        Delete(swigType);
    }
    linkObject += ")";

    return linkObject + additionalObject;
}

void DocXMLConverter::handleTagLink(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string&)
{
    std::string dummy;
    if (!tag.entityList.size())
        return;

    string linkObject = convertLink(tag.entityList.begin()->data);
    if (!linkObject.size())
        linkObject = tag.entityList.begin()->data;
    tag.entityList.pop_front();

    translatedComment += "{@link ";
    translatedComment += linkObject + " ";
    handleParagraph(tag, translatedComment, dummy);
    translatedComment += "}";
}

void DocXMLConverter::handleTagSee(DoxygenEntity& tag,
    std::string& translatedComment,
    std::string&)
{
    std::string dummy;
    if (!tag.entityList.size())
        return;

    // tag.entity list contains contents of the @see paragraph. It should contain
    // one link (references) to method with or without parameters. Doxygen supports
    // arbitrary text and types mixed, but this feature is not supported here.
    // :: or # may be used as a separator between class name and method name.
    list<DoxygenEntity>::iterator it;
    string methodRef;
    for (it = tag.entityList.begin(); it != tag.entityList.end(); it++) {
        if (it->typeOfEntity == "plainstd::endl") {
            // handleNewLine(*it, translatedComment, dummy);
            continue;
        }

        // restore entities which may be used in C++ type declaration
        if (it->typeOfEntity == "&amp") {
            methodRef += '&';
        }
        else if (it->typeOfEntity == "&lt") {
            methodRef += '<';
        }
        else if (it->typeOfEntity == "&gt") {
            methodRef += '>';
        }
        else {
            methodRef += it->data;
        }
    }

    // replace :: with #, but only if it appears before left brace
    size_t lbrace = methodRef.find('(');
    size_t dblColon = methodRef.find("::");
    if (dblColon < lbrace) {
        methodRef = methodRef.substr(0, dblColon) + '#'
            + methodRef.substr(dblColon + 2);
    }

    translatedComment += "@see ";
    string linkObject = convertLink(methodRef);
    if (!linkObject.size()) {
        linkObject = methodRef;
    }
    translatedComment += linkObject;
}

/* This function moves all endlines at the end of child entities
* out of the child entities to the parent.
* For example, entity tree:

-root
|-param
|-paramText
|-endline

should be turned to

-root
|-param
|-paramText
|-endline
*
*/
int DocXMLConverter::shiftEndlinesUpTree(DoxygenEntity &root, int level)
{
    DoxygenEntityListIt it = root.entityList.begin();
    while (it != root.entityList.end()) {
        // remove endlines
        int ret = shiftEndlinesUpTree(*it, level + 1);
        // insert them after this element
        it++;
        for (int i = 0; i < ret; i++) {
            root.entityList.insert(it, DoxygenEntity("plainstd::endl"));
        }
    }

    // continue only if we are not root
    if (!level) {
        return 0;
    }

    int removedCount = 0;
    while (!root.entityList.empty()
        && root.entityList.rbegin()->typeOfEntity == "plainstd::endl") {
        root.entityList.pop_back();
        removedCount++;
    }
    return removedCount;
}

/**
* This makes sure that all comment lines contain '*'. It is not mandatory in doxygen,
* but highly recommended for DocXML. '*' in empty lines are indented according
* to indentation of the first line. Indentation of non-empty lines is not
* changed - garbage in garbage out.
*/
std::string DocXMLConverter::indentAndInsertAsterisks(const string &doc)
{

    size_t idx = doc.find('\n');
    size_t indent = 0;
    // Detect indentation.
    //   The first line in comment is the one after '/**', which may be
    //   spaces and '\n' or the text. In any case it is not suitable to detect
    //   indentation, so we have to skip the first '\n'.
    if (idx != string::npos) {
        size_t nonspaceIdx = doc.find_first_not_of(" \t", idx + 1);
        if (nonspaceIdx != string::npos) {
            indent = nonspaceIdx - idx;
        }
    }

    if (indent == 0) { // we can't indent the first line less than 0
        indent = 1;
    }

    // Create the first line of DocXML comment.
    string indentStr(indent - 1, ' ');
    string translatedStr = indentStr + "/**";
    if (indent > 1) {
        // remove the first space, so that '*' will be aligned
        translatedStr = translatedStr.substr(1);
    }

    translatedStr += doc;

    // insert '*' before each comment line, if it does not have it
    idx = translatedStr.find('\n');

    while (idx != string::npos) {

        size_t nonspaceIdx = translatedStr.find_first_not_of(" \t", idx + 1);
        if (nonspaceIdx != string::npos && translatedStr[nonspaceIdx] != '*') {
            // line without '*' found - is it empty?
            if (translatedStr[nonspaceIdx] != '\n') {
                // add '* ' to each line without it
                translatedStr = translatedStr.substr(0, nonspaceIdx) + "* "
                    + translatedStr.substr(nonspaceIdx);
                //printf(translatedStr.c_str());
            }
            else {
                // we found empty line, replace it with indented '*'
                translatedStr = translatedStr.substr(0, idx + 1) + indentStr + "* "
                    + translatedStr.substr(nonspaceIdx);
            }
        }
        idx = translatedStr.find('\n', nonspaceIdx);
    }

    // Add the last comment line properly indented
    size_t nonspaceEndIdx = translatedStr.find_last_not_of(" \t");
    if (nonspaceEndIdx != string::npos) {
        if (translatedStr[nonspaceEndIdx] != '\n') {
            translatedStr += '\n';
        }
        else {
            // remove trailing spaces
            translatedStr = translatedStr.substr(0, nonspaceEndIdx + 1);
        }
    }
    translatedStr += indentStr + "*/\n";

    return translatedStr;
}

String *DocXMLConverter::makeDocumentation(Node *node)
{

    String *documentation = getDoxygenComment(node);

    if (documentation == NULL) {
        return NewString("");
    }

    if (GetFlag(node, "feature:doxygen:notranslate")) {

        string doc = Char(documentation);

        string translatedStr = indentAndInsertAsterisks(doc);

        String *comment = NewString(translatedStr.c_str());
        // Append(comment, documentation); Replaceall(comment, "\n", "\n * ");
        return comment;
    }

    DoxygenEntityList entityList = parser.createTree(Char(documentation),
        Char(Getfile(documentation)), Getline(documentation));

    // entityList.sort(CompareDoxygenEntities()); sorting currently not used,

    if (debug) {
        std::cout << "---RESORTED LIST---" << std::endl;
        printTree(entityList);
    }

    // store the current node
    // (currently just to handle params)
    currentNode = node;

    std::string docXMLString = "/// ";

    DoxygenEntity root("root", entityList);

    shiftEndlinesUpTree(root);

    // strip endlines at the beginning
    while (!root.entityList.empty()
        && root.entityList.begin()->typeOfEntity == "plainstd::endl") {
        root.entityList.pop_front();
    }

    // and at the end
    while (!root.entityList.empty()
        && root.entityList.rbegin()->typeOfEntity == "plainstd::endl") {
        root.entityList.pop_back();
    }

    docXMLString += translateSubtree(root);

    docXMLString += "\n";

    if (debug) {
        std::cout << "\n---RESULT IN DOCXML---" << std::endl;
        std::cout << docXMLString;
    }

    return NewString(docXMLString.c_str());
}

void DocXMLConverter::addError(int warningType, const std::string &message)
{
    Swig_warning(warningType, "", 0, "Doxygen parser warning: %s. \n",
        message.c_str());
}
