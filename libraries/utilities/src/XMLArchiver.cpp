////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Machine Learning Library (EMLL)
//  File:     XmlArchiver.cpp (utilities)
//  Authors:  Chuck Jacobs
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "XMLArchiver.h"
#include "Archiver.h"
#include "IArchivable.h"

// stl
#include <iostream>
#include <string>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <cassert>

namespace utilities
{
    //
    // Serialization
    //
    XmlArchiver::XmlArchiver() : _out(std::cout) 
    {
        WriteFileHeader();
    }

    XmlArchiver::XmlArchiver(std::ostream& outputStream) : _out(outputStream) 
    {
        WriteFileHeader();
    }

    XmlArchiver::~XmlArchiver()
    {
        WriteFileFooter();
    }

    void XmlArchiver::WriteFileHeader()
    {   
        // Write XML declaration
        _out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";     
        _out << "<emll version=\"1.0\">\n";
    }

    void XmlArchiver::WriteFileFooter()
    {
        _out << "</emll>\n";
    }

    IMPLEMENT_ARCHIVE_VALUE(XmlArchiver, bool);
    IMPLEMENT_ARCHIVE_VALUE(XmlArchiver, char);
    IMPLEMENT_ARCHIVE_VALUE(XmlArchiver, short);
    IMPLEMENT_ARCHIVE_VALUE(XmlArchiver, int);
    IMPLEMENT_ARCHIVE_VALUE(XmlArchiver, size_t);
    IMPLEMENT_ARCHIVE_VALUE(XmlArchiver, float);
    IMPLEMENT_ARCHIVE_VALUE(XmlArchiver, double);

    // strings
    void XmlArchiver::ArchiveValue(const char* name, const std::string& value) { WriteScalar(name, value); }

    // IArchivable
    void XmlArchiver::BeginArchiveObject(const char* name, const IArchivable& value)
    {
        auto indent = GetCurrentIndent();
        auto typeName = XmlUtilities::EncodeTypeName(value.GetRuntimeTypeName());

        _out << indent;
        _out << "<" << typeName;

        if (name != std::string(""))
        {
            _out << " name='" << name << "'";
        }
        _out << ">" << std::endl;
    }

    void XmlArchiver::ArchiveObject(const char* name, const IArchivable& value)
    {
        ++_indent;
        value.WriteToArchive(*this); // TODO: need to somehow know if we're in an indenting context or not for the subsequent calls to WriteScalar
        --_indent;
    }

    void XmlArchiver::EndArchiveObject(const char* name, const IArchivable& value)
    {
        auto indent = GetCurrentIndent();
        auto typeName = XmlUtilities::EncodeTypeName(value.GetRuntimeTypeName());
        _out << indent;
        _out << "</" << typeName << ">" << std::endl;
    }

    //
    // Arrays
    //
    IMPLEMENT_ARCHIVE_ARRAY(XmlArchiver, bool);
    IMPLEMENT_ARCHIVE_ARRAY(XmlArchiver, char);
    IMPLEMENT_ARCHIVE_ARRAY(XmlArchiver, short);
    IMPLEMENT_ARCHIVE_ARRAY(XmlArchiver, int);
    IMPLEMENT_ARCHIVE_ARRAY(XmlArchiver, size_t);
    IMPLEMENT_ARCHIVE_ARRAY(XmlArchiver, float);
    IMPLEMENT_ARCHIVE_ARRAY(XmlArchiver, double);

    void XmlArchiver::ArchiveArray(const char* name, const std::vector<std::string>& array)
    {
        WriteArray(name, array);
    }

    // Array of pointers-to-IArchivable
    // TOOD: pass in compile-time type name
    void XmlArchiver::ArchiveArray(const char* name, const std::string& baseTypeName, const std::vector<const IArchivable*>& array)
    {
        bool hasName = name != std::string("");
        auto indent = GetCurrentIndent();
        auto endOfLine = "\n";

        _out << indent;
        _out << "<Array";
        if (hasName)
        {
            _out << " name='" << name << "'";
        }
        _out << " type='" << baseTypeName <<  "'>" << std::endl;
        ++_indent;
        for (const auto& item : array)
        {
            Archive(*item);
        }
        --_indent;
        _out << indent;
        _out << "</Array>" << endOfLine;
    }

    //
    // Deserialization
    //
    SimpleXmlUnarchiver::SimpleXmlUnarchiver(SerializationContext context) : Unarchiver(std::move(context)), _tokenizer(std::cin, "<>=/'\"")
    {
        ReadFileHeader();
    }

    SimpleXmlUnarchiver::SimpleXmlUnarchiver(std::istream& inputStream, SerializationContext context) : Unarchiver(std::move(context)), _tokenizer(inputStream, "<>?=/'\"") 
    {
        ReadFileHeader(); 
    }

    SimpleXmlUnarchiver::~SimpleXmlUnarchiver() 
    {
        ReadFileFooter(); 
    }

    IMPLEMENT_UNARCHIVE_VALUE(SimpleXmlUnarchiver, bool);
    IMPLEMENT_UNARCHIVE_VALUE(SimpleXmlUnarchiver, char);
    IMPLEMENT_UNARCHIVE_VALUE(SimpleXmlUnarchiver, short);
    IMPLEMENT_UNARCHIVE_VALUE(SimpleXmlUnarchiver, int);
    IMPLEMENT_UNARCHIVE_VALUE(SimpleXmlUnarchiver, size_t);
    IMPLEMENT_UNARCHIVE_VALUE(SimpleXmlUnarchiver, float);
    IMPLEMENT_UNARCHIVE_VALUE(SimpleXmlUnarchiver, double);

    // TODO: add a "read tag"-type function
    void SimpleXmlUnarchiver::ReadFileHeader()
    {   
        _tokenizer.MatchTokens({"<", "?", "xml"});
        while(_tokenizer.PeekNextToken() != "?")
        {
            _tokenizer.ReadNextToken();
        }
        _tokenizer.MatchTokens({"?", ">"});
        _tokenizer.MatchTokens({"<", "emll", "version", "=", "\"", "1.0", "\"", ">"});
    }

    void SimpleXmlUnarchiver::ReadFileFooter()
    {
        _tokenizer.MatchTokens({"<", "/", "emll", ">"});
    }

    // strings
    void SimpleXmlUnarchiver::UnarchiveValue(const char* name, std::string& value) 
    { 
        ReadScalar(name, value); 
    }

    // IArchivable
    std::string SimpleXmlUnarchiver::BeginUnarchiveObject(const char* name, const std::string& typeName) 
    {
        bool hasName = name != std::string("");
        auto rawTypeName = typeName;

        _tokenizer.MatchToken("<");
        auto readTypeName = XmlUtilities::DecodeTypeName(_tokenizer.ReadNextToken());
        assert(readTypeName != "");
        if(hasName)
        {
            _tokenizer.MatchTokens({"name", "=", "'", name, "'"});
        }
        _tokenizer.MatchToken(">");
        return readTypeName;
    }

    void SimpleXmlUnarchiver::UnarchiveObject(const char* name, IArchivable& value) 
    {
        value.ReadFromArchive(*this);
    }

    void SimpleXmlUnarchiver::EndUnarchiveObject(const char* name, const std::string& typeName) 
    {
        auto EncodedTypeName = XmlUtilities::EncodeTypeName(typeName);
        _tokenizer.MatchTokens({"<", "/", EncodedTypeName, ">"});
    }

    //
    // Arrays
    //
    IMPLEMENT_UNARCHIVE_ARRAY(SimpleXmlUnarchiver, bool);
    IMPLEMENT_UNARCHIVE_ARRAY(SimpleXmlUnarchiver, char);
    IMPLEMENT_UNARCHIVE_ARRAY(SimpleXmlUnarchiver, short);
    IMPLEMENT_UNARCHIVE_ARRAY(SimpleXmlUnarchiver, int);
    IMPLEMENT_UNARCHIVE_ARRAY(SimpleXmlUnarchiver, size_t);
    IMPLEMENT_UNARCHIVE_ARRAY(SimpleXmlUnarchiver, float);
    IMPLEMENT_UNARCHIVE_ARRAY(SimpleXmlUnarchiver, double);

    void SimpleXmlUnarchiver::UnarchiveArray(const char* name, std::vector<std::string>& array)
    {
        ReadArray(name, array);
    }

    void SimpleXmlUnarchiver::BeginUnarchiveArray(const char* name, const std::string& typeName)
    {
        bool hasName = name != std::string("");

        _tokenizer.MatchTokens({"<", "Array"});
        if(hasName)
        {
            _tokenizer.MatchTokens({"name", "=", "'", name, "'"});
        }

        _tokenizer.MatchTokens({"type", "=", "'", typeName, "'", ">"});
    }

    bool SimpleXmlUnarchiver::BeginUnarchiveArrayItem(const std::string& typeName)
    {
        // check for '</'
        auto token1 = _tokenizer.ReadNextToken();
        auto token2 = _tokenizer.ReadNextToken();
        _tokenizer.PutBackToken(token2);
        _tokenizer.PutBackToken(token1);
        if(token1+token2 == "</")
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    void SimpleXmlUnarchiver::EndUnarchiveArrayItem(const std::string& typeName)
    {
    }

    void SimpleXmlUnarchiver::EndUnarchiveArray(const char* name, const std::string& typeName)
    {
        _tokenizer.MatchTokens({"<", "/", "Array", ">"});
    }

    //
    // XmlUtilities
    //
    std::string XmlUtilities::EncodeAttributeString(const std::string& str)
    {
        std::vector<char> charCodes(127, '\0');
        charCodes['\''] = '\'';
        charCodes['\"'] = '\"';
        charCodes['\\'] = '\\';
        charCodes['\n'] = 'n';
        charCodes['\r'] = 'r';
        charCodes['\t'] = 't';
        charCodes['\b'] = 'b';
        charCodes['\f'] = 'f';

        // copy characters from str until we hit an escaped character, then prepend it with a backslash
        std::stringstream s;
        for(auto ch: str)
        {
            auto encoding = charCodes[ch];
            if(encoding == '\0') // no encoding
            {
                s.put(ch);
            }
            else
            {
                s.put('\\');
                s.put(encoding);
            }
        }
        return s.str();
    }

    std::string XmlUtilities::DecodeAttributeString(const std::string& str)
    {
        std::vector<char> charCodes(127, '\0');
        charCodes['\''] = '\'';
        charCodes['\"'] = '\"';
        charCodes['\\'] = '\\';
        charCodes['n'] = '\n';
        charCodes['r'] = '\r';
        charCodes['t'] = '\t';
        charCodes['b'] = '\b';
        charCodes['f'] = '\f';

        std::stringstream s;
        bool prevWasBackslash = false;
        for(auto ch: str)
        {
            if(prevWasBackslash)
            {
                auto encoding = charCodes[ch];
                if(encoding == '\0') // nothing special
                {
                    s.put('\\'); // emit previous backslash
                    s.put(ch); // emit character
                }
                else
                {
                    s.put(encoding);
                }
                prevWasBackslash = false;
            }
            else
            {
                if(ch == '\\')
                {
                    prevWasBackslash = true;
                }
                else
                {
                    prevWasBackslash = false;
                    s.put(ch);
                }
            }
        }

        if(prevWasBackslash)
        {
            s.put('\\');
        }
        return s.str();
    }
        std::string XmlUtilities::EncodeTypeName(const std::string& str)
    {
        // convert '<' into '(' etc.
        auto result = str;
        std::replace(result.begin(), result.end(), '<', '(');
        std::replace(result.begin(), result.end(), '>', ')');
        return result;
    }

    std::string XmlUtilities::DecodeTypeName(const std::string& str)
    {
        // convert '(' into '<' etc.
        auto result = str;
        std::replace(result.begin(), result.end(), '(', '<');
        std::replace(result.begin(), result.end(), ')', '>');
        return result;
    }
}
