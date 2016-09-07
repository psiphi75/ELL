////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Machine Learning Library (EMLL)
//  File:     MultiplexerNode.h (node)
//  Authors:  Ofer Dekel
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Node.h"
#include "InputPort.h"
#include "OutputPort.h"

// utilities
#include "TypeName.h"
#include "ObjectArchive.h"

// stl
#include <vector>
#include <memory>
#include <exception>

/// <summary> model namespace </summary>
namespace nodes
{
    /// <summary> A node that outputs a dynamically specified element from an input array. </summary>
    template <typename ValueType, typename SelectorType>
    class MultiplexerNode : public model::Node
    {
    public:
        /// @name Input and Output Ports
        /// @{
        static constexpr const char* elementsPortName = "elements";
        static constexpr const char* selectorPortName = "selector";
        static constexpr const char* outputPortName = "output";
        const model::OutputPort<ValueType>& output = _output;
        /// @}

        /// <summary> Default Constructor </summary>
        MultiplexerNode();

        /// <summary> Constructor </summary>
        ///
        /// <param name="elements"> The input aray of values. </param>
        /// <param name="selector"> The index of the chosen element </param>
        MultiplexerNode(const model::PortElements<ValueType>& elements, const model::PortElements<SelectorType>& selector);

        /// <summary> Gets the name of this type (for serialization). </summary>
        ///
        /// <returns> The name of this type. </returns>
        static std::string GetTypeName() { return utilities::GetCompositeTypeName<ValueType, SelectorType>("MultiplexerNode"); }

        /// <summary> Gets the name of this type (for serialization). </summary>
        ///
        /// <returns> The name of this type. </returns>
        virtual std::string GetRuntimeTypeName() const override { return GetTypeName(); }

        /// <summary> Adds an object's properties to an `Archiver` </summary>
        ///
        /// <param name="archiver"> The `Archiver` to add the values from the object to </param>
        virtual void WriteToArchive(utilities::Archiver& archiver) const override;

        /// <summary> Sets the internal state of the object according to the archiver passed in </summary>
        ///
        /// <param name="archiver"> The `Archiver` to get state from </param>
        virtual void ReadFromArchive(utilities::Unarchiver& archiver) override;

        /// <summary> Makes a copy of this node in the model being constructed by the transformer </summary>
        virtual void Copy(model::ModelTransformer& transformer) const override;

    protected:
        virtual void Compute() const override;

    private:
        // Inputs
        model::InputPort<ValueType> _elements;
        model::InputPort<SelectorType> _selector;

        // Output
        model::OutputPort<ValueType> _output;
    };
}

#include "../tcc/MultiplexerNode.tcc"
