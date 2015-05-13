/* 
 * This library converts everything to boost::property_tree::ptree as an
 * intermediate format. Properties are inherently hierarchical.
 */

/*
 * Schema:
 * We need one, but I don't know what it should look like. In general it
 * needs to provide
 * - names and aliases of properties
 * - "types" of properties. buitin types, arrays, associative arrays,
 *   user types?
 * - documentation
 * - merge policy - concatenation, override
 * - conflict/dependencies
 * - default values
 *
 * I would suggest that the schema should not specify anything which is source
 * dependent, e.g. special handling for positional args for commandline
 * parsing
 */
class Schema
{
    // some stuff here


    /* Design questions
     * - should Schema print documentation, or should some other
     *   function/class use it to print the documentation
     * - should Schema merge ptrees from different input sources or should
     *   some other class use it to do the merge.
     */
};

/* There needs to be some way to uniquely identify a property defined within
 * the schema. Since we are representing properties as trees, I propose some
 * sort of path for traversing the tree.
 *
 * ... This should maybe be a member of Schema
 */
class SchemaPath
{
    // some code here
};

/* The user interface class wraps up the functionality of all the others so
 * that a user can simply instantiate and go
 */
class ConfigParser
{
public:
    /* construct the parser with a fixed schema
     */
    ConfigParser(const Schema & schema);

    /* add input sources. The order in which they are added defines their
     * priority during a merge.
     */
    ConfigParser & input(ConfigSource & source);

    /* Reads all the data in from the sources and checks if it is valid.
     * If the schema marks a property as cacheable, this will cache that value
     * for later use in validate() or getProperty()
     *
     * How does it handle errors? Is this what prints out the usage error to
     * the command line? Then this becomes command line specific
     */
    bool validate();

    /* Gets the ptree representing the property. If lazy evaluation is
     * allowed, this may evaluate all the ConfigSources. If the schema marks
     * this property as cacheable, this will use a cached value or cache
     * whatever value it computes
     *
     * ? how does it report errors
     */
    const pt::ptree & getProperty(const SchemaPath & path);

    /* write the current configuration out
     */
    void write(ConfigSink & sink);
    // write a subset of the current configuration
    void write(ConfigSink & sink, const std::vector<SchemaPath> & properties);
};

/* 
 * ConfigSource is the abstract base class for parsers which read from
 * arbitrary sources of config data. It's purpose is to convert from the input
 * config source -- file, database, command line -- to a ptree which conforms
 * to the schema structure. 
 *
 * To enable expanding the library, the ConfigSources use Validators which
 * validate a chunk of input conforms to the "type" specified in the schema.
 *
 * Could consider instead passing the schema to the constructor. Could also
 * consider passing auxilliary data such as how to handle position arguments
 * to the constructor.
 */
class ConfigSource
{
public:
    /* Read the configuration data from this source.
     *
     * - how does it report errors?
     */
    virtual pt::ptree && read(const Schema & schema) = 0;
    

    /* if we want to support fields defined in terms of other fields we need
     * to provide the values read so far from other sources. This also allows
     * chaining of ConfigSources. This function needs to merge the initial
     * ptree with the data read from the source using Schema (or whatever
     * class does merge)
     */
    virtual pt::ptree && read(const Schema & schema, const pt::ptree & init) =
        0;

};

/* Validators are domain specific. They consume some format specific data
 * type, and return a ptree representing the data. Each validator coresponds
 * to one "type" in the schema. The library should provide implementations for
 * built-in types, string, array, associative array. Users can provide
 * specializations for their own types.
 */
namespace CommandLine
{
    template <typename T>
    class Validator
    {
    public:
        // return a ptree containing a T or an empty ptree if it didn't
        // parse
        pt::ptree && operator() (const char * arg);
    };
}

/* The opposite of a ConfigSource, ConfigSink writes a configuration back out.
 * Many implementations will probably inherit from both ConfigSource and
 * ConfigSink. The command line is a notable exception that will probably only
 * implement ConfigSource.
 */
class ConfigSink
{
public:
    virtual void write(const pt::ptree & config, const Schema & schema) = 0;
};


/****** Add Converters and hide usage of ptree **********
 * If we get to it, it would be good to hide the use of ptree from the user
 * and instead give them descriptive C++ types. 
 *
 * This would require adding the actual type to the Schema and modifying
 * ConfigParser's get function
 */
class ConfigParserBetter: public ConfigParser
{
public:
    template <typename T>
        T && get(const SchemaPath & path) const;
};

/* under the hood uses Converter class. Users can specialize Converter for
 * their own types. 
 */
template <typename T>
class Converter 
{
public:
    /* convert a tree which should represent this type to the actual type
     * (maybe should be static const)
     */
    T && fromPtree(const pt::ptree & tree);
};
