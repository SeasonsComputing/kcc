/*
 * Kuumba C++ Core
 *
 * $Id: Properties.cpp 21993 2008-02-05 18:55:20Z tvk $
 */
#include <inc/core/Core.h>

#define KCC_FILE "Properties"

namespace kcc
{
    // XML tags
    static const String k_xmlRoot    (KCC_PROPERTY_XML_PROPERTIES);   
    static const String k_xmlProperty(KCC_PROPERTY_XML_PROPERTY);   
    static const String k_xmlKey     (KCC_PROPERTY_XML_PROPERTYKEY);   
    static const String k_xmlValue   (KCC_PROPERTY_XML_PROPERTYVALUE);

    // Constants
    static const String k_cmdValueSep("=");

    // Properties
    static const String k_keyConfigFile (KCC_PROPERTY_FILE);
    static const String k_keyAppName    (KCC_APPLICATION_NAME);
    
    // k_overrideProperties: override properties
    inline static void k_overrideProperties(StringMap& target, const StringMap& source)
    {
        for (StringMap::const_iterator i = source.begin(); i != source.end(); i++) 
            target[i->first] = i->second;
    }
    
    // k_chainProperties: chain properties files
    static bool k_chainProperties(Properties& p)
    {
        if (!p.exists(k_keyConfigFile)) return true;
        String path(p.get(k_keyConfigFile, Strings::empty()));
        p.erase(k_keyConfigFile); // prevent circular loading
        if (!p.load(path, false)) return false;
        return true;
    }
    
    // k_parsePropertyParam: parse and set param (key=value) into properties
    static void k_parsePropertyParam(const String& param, Properties& p)
    {
        String::size_type sep = param.find(k_cmdValueSep);
        if (sep == String::npos) p.set(param, KCC_PROPERTY_TRUE);
        else                     p.set(param.substr(0, sep), param.substr(sep+1));
    }

    // k_propertyAttribute: get property attribute from xml Property line
    static String k_propertyAttribute(const String& line, const String& key)
    {
        String::size_type kpos = line.find(key);
        if (kpos == String::npos) return Strings::empty();
        
        // find value between like-quotes
        String value;
        String::size_type eol = line.size();
        String::size_type i   = kpos + key.size();
        Char q = 0;
        while (i < eol)
        {
            Char c = line[i];
            
            // found end of value
            if (q != 0 && c == q) break;
            
            // look for start of value or add to value
            if (q == 0 && (c == '\'' || c == '\"'))
                q = c;
            else if (q != 0)
                value += c;
                
            i++;
        }
        
        return Strings::xmlDecode(value);
    }

    // ctor/dtor
    Properties::Properties() {}
    Properties::Properties(const Properties& properties) { *this = properties; }
    Properties::~Properties() {}

    // copy ctor
    Properties& Properties::operator = (const Properties& rhs)
    {
        if (this != &rhs) 
        {
            m_properties.clear();
            m_properties.insert(rhs.m_properties.begin(), rhs.m_properties.end());
        }
        return *this;
    }

    // clear: clear properties
    void Properties::clear() { m_properties.clear(); }

    // load: load from vector (each vector: key=value)
    bool Properties::load(const StringVector& args, bool clear)
    {
        // load properties
        Properties load;
        for (StringVector::size_type i = 0; i < args.size(); i++) k_parsePropertyParam(args[i], load);
        Properties save(load);
    
        // overrides
        if (!k_chainProperties(load)) return false;
        if (clear) m_properties.clear();
        k_overrideProperties(m_properties, load.m_properties);
        k_overrideProperties(m_properties, save.m_properties);
        return true;
    }

    // load: load from command line args
    bool Properties::load(int argc, const char* argv[], bool clear)
    {
        // load properties
        Properties load;
        load.set(k_keyAppName, argv[0]);
        for (int i = 1; i < argc; i++) k_parsePropertyParam(argv[i], load);        
        Properties save(load);
    
        // overrides
        if (!k_chainProperties(load)) return false;
        if (clear) m_properties.clear();
        k_overrideProperties(m_properties, load.m_properties);
        k_overrideProperties(m_properties, save.m_properties);
        return true;
    }

    // load: load properties from a file
    bool Properties::load(const String& path, bool clear)
    {
        std::ifstream in(path.c_str());
        return fromXML(in, clear);
    }

    // load: intialize from file
    bool Properties::save(const String& path) const
    {
        std::ofstream out;
        out.open(path.c_str());
        if (!out.good()) return false;
        bool ok = toXML(out);
        out.close();
        return ok;
    }

    // fromXML: read properties from xml stream
    bool Properties::fromXML(std::istream& in, bool clear)
    {
        // NOTE:
        // * properties loading mostly happens pre-Core::init() so we can't use
        //   much of kcc (Regex, Log, RODOM, etc.) so we fake XML parsing
        // * this is a highly specialized and simplistic parser to handle the 
        //   specific format for Properties XML (due to pre-Core::init() use)

        // load properties
        if (!in.good()) return false;
        Properties load;
        bool entity = false, record = false, comment = false;
        String line;
        Char c = 0;
        in.read(&c, sizeof(Char));
        while (!in.eof())
        {
            if (comment)
            {
                line += c;
                if (line.substr(line.size() - 3) == "-->")
                {
                    // close comment
                    comment = false;
                    
                    // reset parse state
                    entity = record = comment = false;
                    line.clear();
                }
            }
            else if (line.size() == 4 && line.substr(0, 4) == "<!--")
            {
                // open comment
                comment = true;
            }
            else 
            {
                if (c == '<')
                {
                    // open entity
                    if (record) return false; // already recording
                    record = true;
                }
                else if (c == '>')
                {
                    // close entity
                    if (!record) return false; // not recording
                    entity = true;
                }
                if (record) line += c;
                if (entity)
                {
                    // parse property
                    if (line.find(k_xmlProperty) != String::npos)
                    {
                        String k(k_propertyAttribute(line, k_xmlKey));
                        String v(k_propertyAttribute(line, k_xmlValue));
                        if (k.empty()) return false;
                        load.set(k, v);
                    }
                    
                    // reset parse state
                    entity = record = comment = false;
                    line.clear();
                }
            }

            in.read(&c, sizeof(Char));
        }
        Properties save(load);

        // overrides
        if (!k_chainProperties(load)) return false;
        if (clear) m_properties.clear();
        k_overrideProperties(m_properties, load.m_properties);
        k_overrideProperties(m_properties, save.m_properties);
        return true;
    }

    // toXML: serialize to stream as XML
    bool Properties::toXML(std::ostream& out, bool noPrologue) const
    {
        DOMWriter w(out, noPrologue);
        w.start(k_xmlRoot);
        for (
            StringMap::const_iterator i = m_properties.begin();
            i != m_properties.end();
            i++)
        {
            w.start(k_xmlProperty);
            w.attr(k_xmlKey,   i->first);
            w.attr(k_xmlValue, i->second);
            w.end(k_xmlProperty);
        }
        w.end(k_xmlRoot);
        return true;
    }

    // exists: query if key exists
    bool Properties::exists(const String& key) const
    {
        StringMap::const_iterator v = m_properties.find(Strings::trimws(key));
        return v != m_properties.end();
    }

    // erase: erase property
    bool Properties::erase(const String& key)
    {
        StringMap::iterator f = m_properties.find(Strings::trimws(key));
        bool exists = f != m_properties.end();
        if (exists) m_properties.erase(f);
        return exists;
    }
    
    // get: fetch value for key
    const String& Properties::get(const String& key, const String& value) const
    {
        StringMap::const_iterator v = m_properties.find(Strings::trimws(key));
        return (v == m_properties.end()) ? value : v->second;
    }

    // get: fetch value for key
    long Properties::get(const String& key, long value) const
    {
        const String& v = get(key, Strings::empty());
        return (v.empty()) ? value : std::atol(v.c_str());
    }

    // get: fetch value for key
    double Properties::get(const String& key, double value) const
    {
        const String& v = get(key, Strings::empty());
        return (v.empty()) ? value : std::atof(v.c_str());
    }

    // set: fetch value for key
    void Properties::set(const String& key, const String& value)
    {
        m_properties[Strings::trimws(key)] = Strings::trimws(value);
    }

    // set: fetch value for key
    void Properties::set(const String& key, long value)
    {
        set(key, Strings::printf("%d", value));
    }

    // set: fetch value for key
    void Properties::set(const String& key, double value)
    {
        set(key, Strings::printf("%f", value));
    }

    // keys: fetch properties keys
    void Properties::keys(StringVector& keys, const String& prefix) const
    {
        keys.clear();
        keys.reserve(m_properties.size());
        bool all = prefix.empty();
        for (
            StringMap::const_iterator i = m_properties.begin();
            i != m_properties.end();
            i++)
        {
            if (all || i->first.find(prefix) == 0) keys.push_back(i->first);
        }
    }

    // keys: fetch properties keys
    void Properties::keys(StringSet& keys, const String& prefix) const
    {
        keys.clear();
        bool all = prefix.empty();
        for (
            StringMap::const_iterator i = m_properties.begin();
            i != m_properties.end();
            i++)
        {
            if (all || i->first.find(prefix) == 0) keys.insert(i->first);
        }
    }

    // asMap: properties as map
    void Properties::asMap(StringMap& map, bool clear) const
    {
        if (clear) map.clear();
        map = m_properties;
    }

    // empty: empty properties object
    const Properties& Properties::empty()
    {
        static const Properties k_empty;
        return k_empty;
    }
}
