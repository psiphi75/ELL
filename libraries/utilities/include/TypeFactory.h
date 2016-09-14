////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Machine Learning Library (EMLL)
//  File:     TypeFactory.h (utilities)
//  Authors:  Ofer Dekel
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Exception.h"

// stl
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace emll
{
namespace utilities
{
    /// <summary>
    /// Assume that Base is a polymorpic base type, with multiple derived classes. Both the base type
    /// and the derived types must have a public static member named GetTypeName(), which returns the
    /// class name. Namely,
    ///
    /// class Base {public: static std::string GetTypeName(){ return "Base";}};
    /// class Derived1 : public Base {public: static std::string GetTypeName(){ return "Derived1";}};
    /// class Derived2 : public Base {public: static std::string GetTypeName(){ return "Derived2";}};
    ///
    /// The type factory lets the user construct instances of the derived classes by specifying their
    /// names. First, we must create a factory.
    ///
    /// TypeFactory&lt;Base&gt; factory;
    ///
    /// Note that the class template type defines the type of unique_ptr generated by the factory.
    /// Next, we register the derived classes that we want to construct.
    ///
    /// factory.AddType&lt;Derived1&gt;();
    /// factory.AddType&lt;Derived2&gt;();
    ///
    /// Now, we can construct instances of each class.
    ///
    /// auto x = factory.Construct("Derived1");
    ///
    /// x is a unique_ptr&lt;Base&gt; that points to an instance of the Derived1 class.
    /// </summary>
    ///
    /// <typeparam name="BaseType"> Type of the polymorphic base type. </typeparam>
    template <typename BaseType>
    class TypeFactory
    {
    public:
        /// <summary> Invokes the default constructor of the named type. </summary>
        ///
        /// <param name="typeName"> Name of the type to construct. </param>
        ///
        /// <returns> A std::unique_ptr to the base type, which points to the newly constructed object </returns>
        std::unique_ptr<BaseType> Construct(const std::string& typeName) const;

        /// <summary> Adds a type to the factory, with its default name. </summary>
        ///
        /// <typeparam name="RuntimeType"> Type being added. </typeparam>
        template <typename RuntimeType>
        void AddType();

        /// <summary> Adds a type to the factory, with its default name. </summary>
        ///
        /// <typeparam name="RuntimeType"> Type being added. </typeparam>
        /// <param name="typeName"> Name of the type being added. </param>
        template <typename RuntimeType>
        void AddType(const std::string& typeName);

    private:
        std::unordered_map<std::string, std::function<std::unique_ptr<BaseType>()>> _typeMap;
    };

    /// <summary>
    /// An internal class used by GenericTypeFactory
    /// </summary>
    class TypeConstructorBase
    {
    public:
        virtual ~TypeConstructorBase() = default;

    private:
        friend class GenericTypeFactory;
        template <typename BaseType>
        friend class TypeConstructor;
        template <typename BaseType>
        std::unique_ptr<BaseType> Construct() const;
    };

    /// <summary>
    /// A factory object that can create new objects given their type name and a base type to derive from.
    /// </summary>
    class GenericTypeFactory
    {
    public:
        /// <summary> Invokes the default constructor of the named type. </summary>
        ///
        /// <typeparam name="BaseeType"> The base type to return. </typeparam>
        /// <param name="typeName"> Name of the type to construct. </param>
        ///
        /// <returns> A std::unique_ptr to the base type, which points to the newly constructed object </returns>
        template <typename BaseType>
        std::unique_ptr<BaseType> Construct(const std::string& typeName) const;

        /// <summary> Adds a type to the factory, with its default name. </summary>
        ///
        /// <typeparam name="RuntimeType"> Type being added. </typeparam>
        template <typename BaseType, typename RuntimeType>
        void AddType();

        /// <summary> Adds a type to the factory, with its default name. </summary>
        ///
        /// <typeparam name="RuntimeType"> Type being added. </typeparam>
        /// <param name="typeName"> Name of the type being added. </param>
        template <typename BaseType, typename RuntimeType>
        void AddType(const std::string& typeName);

    private:
        std::unordered_map<std::string, std::shared_ptr<TypeConstructorBase>> _typeConstructorMap;
    };
}
}

#include "../tcc/TypeFactory.tcc"
