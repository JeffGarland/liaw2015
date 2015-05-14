/**
 *  \file confoost.hpp
 *
 *  Copyright 2015 Jeff Garland
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_CONFOOST_CONFOOST_HPP
#define BOOST_CONFOOST_CONFOOST_HPP

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <boost/any.hpp>

using retrieve_t = std::function< std::tuple<bool, boost::any>(boost::any, std::string) >;

// This is what we discussed as common representation (schema whatever) of our configuration
class confoost_config {
private:
    bool is_valid;
    boost::any options;

    retrieve_t retriever;

public:
    confoost_config() : is_valid(false) {
        retriever = [] (boost::any, std::string path) { return std::make_tuple(false, boost::any()); };
    };

    bool valid() const { return is_valid; };
    void validate() { is_valid = true; };
    void invalidate() { is_valid = false; };

    void set(confoost_config&& source) {
        options = std::move(source.options);
        set_retriever(source.retriever);
        is_valid = true;
    }

    void set_retriever(retrieve_t r) { retriever = r; };

    template<typename T>
    std::tuple<bool, T> retrieve(std::string path) const {
        std::cout << "Retrieving: " << path << std::endl;
        if(is_valid) {
            return std::make_tuple(false, T());/* TODO: add boost::any to T cast here */ // retriever(options, path);
        }
        return std::make_tuple(false, T());
    };
};


// A parser is a function that can return a common configuration object from any source
template<typename C>
using parser_t = std::function<C()>;


// A confoost object is configuration manager that stores many sources (parsers with caches) and it forwards lookups by priority
template<typename C>
class confoost {

    // A source is a parser that captures its cache
    using source_t = std::function<C&()>;

private:
    // Sources are prioritized by index
    std::vector<source_t> sources;

public:
    // We can add as many parsers as we want to our configuration manager
    source_t& add_parser(parser_t<C> parse) {
        // Every new parser become a source with an empty cache
        auto cache = std::make_shared<C>();

        // Parser parameters are captured so that in the end all parsers can share the same signature
        source_t source = [=] () -> C& {
            if(cache->valid()) return *cache;

            cache->set(parse());
            return *cache;
        };

        sources.emplace_back(source);

        // We expose our newly created source to be able to invalidate it outside
        return sources.back();
    };

    // When we look for a key, we ask our sources
    template<typename T>
    T retrieve(std::string path) const {
        for (auto&& get_source : sources) {
            auto source = get_source();
            auto response = source. template retrieve<T> (path);

            // As soon as we find a result, we return it
            if(std::get<0>(response)) return std::get<1>(response);
        }
        // TODO : throw something or report error somehow
        return T();
    }

};

// TODO : implement this
template<typename T>
parser_t<T> xml_parser(std::string filename) {
    return [=]() {
        std::cout << "Parsing XML file: " << filename << std::endl;
        auto config = T();
        config.set_retriever([] (boost::any tree, std::string path) -> std::tuple<bool, boost::any> {
            return std::make_tuple(false, T());
        });
        return config;
    };
};

// TODO : implement this
template<typename T>
parser_t<T> json_parser(std::string filename) {
    return [=]() {
        std::cout << "Parsing JSON file: " << filename << std::endl;
        auto config = T();
        config.set_retriever([] (boost::any tree, std::string path) -> std::tuple<bool, boost::any> {
            return std::make_tuple(false, T());
        });
        return config;
    };
};

// TODO : implement this
template<typename T>
parser_t<T> ini_parser(std::string filename) {
    return [=]() {
        std::cout << "Parsing INI file: " << filename << std::endl;
        auto config = T();
        config.set_retriever([] (boost::any tree, std::string path) -> std::tuple<bool, boost::any> {
            return std::make_tuple(false, T());
        });
        return config;
    };
};

// TODO : implement this
template<typename T>
parser_t<T> cli_parser(const int& argc, char** argv) {
    return [&]() {
        std::cout << "Parsing command line..." << std::endl;
        auto config = T();
        config.set_retriever([] (boost::any tree, std::string path) -> std::tuple<bool, boost::any> {
            return std::make_tuple(false, T());
        });
        return config;
    };
};

#endif // BOOST_CONFOOST_CONFOOST_HPP



/************* USAGE ************* (can be improved by stating default template arguments in this hpp)

#include "confoost.hpp" 

using schema = confoost_config<confoost_node>;
using configuration = confoost<schema>;

int main(int argc, char** argv) {
    auto config = confoost<confoost_config<confoost_node>>();

    auto xml_source = config.add_parser( xml_parser<schema>("config.xml") );
    auto json_source = config.add_parser( json_parser<schema>("config.json") );
    auto cli_source = config.add_parser( cli_parser<schema>(argc, argv) ) ;

    auto a_value = config.retrieve<int>("some/path/to/some/int");

    std::cout << std::endl;

    a_value = config.retrieve<int>("some/path/to/some/int");
    //xml_source().invalidate();
    //a_value = config.retrieve<int>("some/path/to/some/int");

    return 0;
}

*/