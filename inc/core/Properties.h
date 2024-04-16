/*
 * Kuumba C++ Core
 *
 * $Id: Properties.h 22156 2008-02-20 14:34:22Z tvk $
 */
#ifndef Properties_h
#define Properties_h

namespace kcc
{
    /**
    * Class to manage key/value properties ADT
    *
    * XML:
    *   <Properties>
    *        <Property key='foo' value='bar' />
    *        ...
    *   </Properties>
    *
    * @author Ted V. Kremer
    */
    class KCC_CORE_EXPORT Properties
    {
    public:
        /** Properties ctor/dtor */
        Properties();
        Properties(const Properties& properties);
        Properties& operator = (const Properties&);
        virtual ~Properties();

        /**
         * Clears existing properties
         */
        void clear();

        /**
         * Load properties from a file
         * @param path to load from
         * @param clear clear this object's properties prior to loading
         * @return true if loaded 
         */
        bool load(const String& path, bool clear = true);

        /**
         * Load properties from vector {key}={value} rows
         * @param args rows of key/value pairs
         * @param clear clear this object's properties prior to loading
         * @return true if loaded 
         */
        bool load(const StringVector& args, bool clear = true);

        /**
         * Load properties from command line args
         * @param argc command line arg count
         * @param argv command line args
         * @param clear clear this object's properties prior to loading
         * @return true if loaded 
         */
        bool load(int argc, const char* argv[], bool clear = true);

        /**
         * Save properties to a file
         * @param path to save to
         * @return true if saved
         */
        bool save(const String& path) const;
        
        /**
         * Deserialize properties from an xml stream
         * @param in xml stream
         * @param clear clear this object's properties prior to loading
         * @return true if deserialized
         */
        bool fromXML(std::istream& in, bool clear = true);

        /**
         * Serialize properties to XML
         * @param out stream to serialize to
         * @param noPrologue don't emit an xml prologue
         * @return true if written
         */
        bool toXML(std::ostream& out, bool noPrologue = false) const;

        /**
         * Query if key exists
         * @param key key to check for
         * @return true if key is set
         */
        bool exists(const String& key) const;

        /**
         * Remove if key exists
         * @param key key to erase
         * @return true if found and erased
         */
        bool erase(const String& key);

        /**
         * Query for value (init must be called prior to use)
         * @param key key of value
         * @param value default value
         */
        const String& get(const String& key, const String& value) const;

        /**
         * Query for value (init must be called prior to use)
         * @param key key of value
         * @param value default value
         */
        long get(const String& key, long value) const;

        /**
         * Query for value (init must be called prior to use)
         * @param key key of value
         * @param value default value
         */
        double get(const String& key, double value) const;

        /**
         * Set a key and value (init must be called prior to use)
         * @param key key of value
         * @param value value
         */
        void set(const String& key, const String& value);

        /**
         * Set a key and value (init must be called prior to use)
         * @param key key of value
         * @param value value
         */
        void set(const String& key, long value);

        /**
         * Set a key and value (init must be called prior to use)
         * @param key key of value
         * @param value value
         */
        void set(const String& key, double value);

        /**
         * Get all properties keys
         * @param keys out value containing keys
         * @param prefix prefix of keys to match, empty == *
         */
        void keys(StringVector& keys, const String& prefix = Strings::empty()) const;
        void keys(StringSet& keys, const String& prefix = Strings::empty()) const;

        /**
         * Query contents as string map
         * @param map out-param map to populate
         * @param clear clear out-param map
         */
        void asMap(StringMap& map, bool clear = true) const;

        /**
         * Empty properties collection (GOF: Singleton, Null-Object)
         * @return empty properties
         */
        static const Properties& empty();

    protected:
        // Attributes
        StringMap m_properties;
    };
}

/**
 * True/False conversion for properties 
 */
#define KCC_PROPERTY_TRUE  ((long)true)
#define KCC_PROPERTY_FALSE ((long)false)

/**
 * Property XML
 */
#define KCC_PROPERTY_XML_PROPERTIES    "Properties"
#define KCC_PROPERTY_XML_PROPERTY      "Property"
#define KCC_PROPERTY_XML_PROPERTYKEY   "key"
#define KCC_PROPERTY_XML_PROPERTYVALUE "value"

/**
 * Property configurations
 */
#define KCC_PROPERTY_FILE "kcc.propConfigPath"


#endif // Properties_h
