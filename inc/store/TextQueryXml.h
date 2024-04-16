/**
 * GENERATED FILE: 2007-05-11
 */
#ifndef TextQueryXml_h
#define TextQueryXml_h

namespace kcc
{
    struct TextQueryXml
    {
        static const String& root()
        {
            static const String k_root("TextQuery");
            return k_root;
        }
        static const String& rootService()
        {
            static const String k_rootService("service");
            return k_rootService;
        }
        static const String& rootWhen()
        {
            static const String k_rootWhen("when");
            return k_rootWhen;
        }
        static const String& rootDocuments()
        {
            static const String k_rootDocuments("total");
            return k_rootDocuments;
        }
        static const String& rootIndex()
        {
            static const String k_rootIndex("index");
            return k_rootIndex;
        }
        static const String& rootMaxCursors()
        {
            static const String k_rootMaxCursors("maxCursors");
            return k_rootMaxCursors;
        }
        static const String& rootCursors()
        {
            static const String k_rootCursors("cursors");
            return k_rootCursors;
        }
        static const String& rootExpire()
        {
            static const String k_rootExpire("expire");
            return k_rootExpire;
        }
        static const String& rootMessage()
        {
            static const String k_rootMessage("message");
            return k_rootMessage;
        }
        static const String& rootError()
        {
            static const String k_rootError("error");
            return k_rootError;
        }
        static const String& document()
        {
            static const String k_document("Document");
            return k_document;
        }
        static const String& documentRow()
        {
            static const String k_documentRow("row");
            return k_documentRow;
        }
        static const String& text()
        {
            static const String k_text("Text");
            return k_text;
        }
        static const String& match()
        {
            static const String k_match("Match");
            return k_match;
        }
        static const String& matchStartOffset()
        {
            static const String k_matchStartOffset("s");
            return k_matchStartOffset;
        }
        static const String& matchEndOffset()
        {
            static const String k_matchEndOffset("e");
            return k_matchEndOffset;
        }
        static const String& metadata()
        {
            static const String k_metadata("Metadata");
            return k_metadata;
        }
        static const String& metadataKey()
        {
            static const String k_metadataKey("k");
            return k_metadataKey;
        }
        static const String& metadataValue()
        {
            static const String k_metadataValue("v");
            return k_metadataValue;
        }
        static const String& term()
        {
            static const String k_term("Term");
            return k_term;
        }
        static const String& termTerm()
        {
            static const String k_termTerm("t");
            return k_termTerm;
        }
        static const String& termFrequency()
        {
            static const String k_termFrequency("f");
            return k_termFrequency;
        }
        static const String& status()
        {
            static const String k_status("Status");
            return k_status;
        }
        static const String& statusId()
        {
            static const String k_statusId("id");
            return k_statusId;
        }
        static const String& statusExpression()
        {
            static const String k_statusExpression("expr");
            return k_statusExpression;
        }
        static const String& statusContents()
        {
            static const String k_statusContents("contents");
            return k_statusContents;
        }
        static const String& statusSize()
        {
            static const String k_statusSize("size");
            return k_statusSize;
        }
        static const String& statusRow()
        {
            static const String k_statusRow("row");
            return k_statusRow;
        }
        static const String& statusTotal()
        {
            static const String k_statusTotal("total");
            return k_statusTotal;
        }
        static const String& statusAccessed()
        {
            static const String k_statusAccessed("accessed");
            return k_statusAccessed;
        }
        static const String& statusExpired()
        {
            static const String k_statusExpired("expired");
            return k_statusExpired;
        }
        static const String& time()
        {
            static const String k_time("time");
            return k_time;
        }

    private:
        TextQueryXml();
    };
}

#endif // TextQueryXml_h
