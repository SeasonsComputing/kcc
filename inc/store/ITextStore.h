/*
 * Kuumba C++ Core
 *
 * $Id: ITextStore.h 22760 2008-03-24 15:51:02Z tvk $
 */
#ifndef ITextStore_h
#define ITextStore_h

namespace kcc
{
    /** Text Exception */
    KCC_COMPONENT_EXCEPTION(TextException);

    /**
     * Text document
     */
    struct TextDocument
    {
        /** Text data structures */
        typedef std::map<String, long> Terms;
        enum ContentsFlags 
        { 
            C_METADATA      = 1, // store/return metadata
            C_TEXT          = 2, // store/return document text (allows using the term-index and boolean query facilities only)
            C_TERMS         = 4, // store/return tokenized term frequency vector
            C_QUERY_MATCHES = 8  // return document query selections
        };
        typedef unsigned int Contents;
        enum MetadataFlags
        {
            MD_TEXT, // index metadata as tokenized terms (default; boolean query; foo AND bar NOT zap)
            MD_ENUM  // index metadata as exact value ('20071010')
        };
        typedef std::map<String, MetadataFlags> MetadataFields;
        struct Match
        {
            long startOffset;
            long endOffset;
            Match(long so = -1L, long eo = -1L) : startOffset(so), endOffset(eo) {}
        };
        typedef std::vector<Match> Matches;

        // Attributes
        StringMap metadata;
        String    text;
        Terms     terms;
        Matches   matches;

        // Modifiers
        void clear() 
        { 
            metadata.clear(); 
            text.clear(); 
            terms.clear();
            matches.clear();
        }
    };
    typedef std::vector<TextDocument> TextDocuments;

    /**
     * Text results
     *
     * @author Ted V. Kremer
     */
    interface ITextResults : IComponent
    {
        /**
         * Query if more results available
         * @return true if results available
         * @throws TextException if text error
         */
        virtual bool next() throw (TextException) = 0;

        /**
         * Seek to row
         * @param row row to position cursor at
         * @throws TextException if text error
         */
        virtual void seek(long row) throw (TextException) = 0;

        /**
         * Results totals
         */
        virtual long total() = 0;
        virtual long row  () = 0;

        /**
         * Fetch results
         * @param txtdoc out-param of document results
         * @throws TextException if text error
         */
        virtual void results(TextDocument& txtdoc) throw (TextException) = 0;
    };

    /**
     * Text query
     *
     * @author Ted V. Kremer
     */
    interface ITextQuery : IComponent
    {
        /**
         * Text query properties
         * @param config text query configuration
         * @return true if initialized
         */
        virtual bool init(const Properties& config) = 0;

        /**
         * Query text from store
         * @param query expression
         * @param contents OR'd attr's of what contents to populate 
         * @return results (ownership IS consumed)
         * @throws TextException if text error
         */
        virtual ITextResults* query(const String& expression, TextDocument::Contents contents = TextDocument::C_METADATA) throw (TextException) = 0;
    };

    /**
     * Text store (indexing & query facilities)
     *
     * @author Ted V. Kremer
     */
    interface ITextStore : ITextQuery
    {
        /**
         * Text component properties
         * @param config text store configuration
         * @return true if initialized
         */
        virtual bool init(const Properties& config) = 0;
        virtual const String& indexRepository() = 0;

        /**
         * Query text from store
         * @param query expression
         * @param contents OR'd attr's of what contents to populate 
         * @return results (ownership IS consumed)
         * @throws TextException if text error
         */
        virtual ITextResults* query(const String& expression, TextDocument::Contents contents = TextDocument::C_METADATA) throw (TextException) = 0;

        /**
         * Index text into store
         * @param document txtdoc to index (document.text required, document.metadata optional, termCount ignored)
         * @param contents document contents (C_METADATA to index metadata, 
         *                 C_TEXT to store complete text in index, C_TERMS to store tokenized term frequency vectors)
         * @throws TextException if text error
         */
        virtual void indexText(
            TextDocument&                       txtdoc, 
            const TextDocument::MetadataFields& fields,
            TextDocument::Contents              contents) throw (TextException) = 0;
        virtual void indexText     (TextDocument& txtdoc, TextDocument::Contents contents = TextDocument::C_TEXT|TextDocument::C_METADATA) throw (TextException) = 0;
        virtual long indexDocuments() throw (TextException) = 0;
        virtual void indexOpen     () throw (TextException) = 0;
        virtual bool indexIsOpen   () throw (TextException) = 0;
        virtual void indexOptimize () throw (TextException) = 0;
        virtual void indexClose    () throw (TextException) = 0;
    };
}

#endif // ITextStore_h
