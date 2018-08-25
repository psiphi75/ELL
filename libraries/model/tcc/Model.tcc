////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Learning Library (ELL)
//  File:     Model.tcc (model)
//  Authors:  Chuck Jacobs
//
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ell
{
namespace model
{
    namespace detail
    {
        class ModelNodeRouter
        {
        public:
            template <typename T>
            static T&& ConvertPortElementsArgImpl(Model& model, T&& arg, std::false_type, bool)
            {
                // pass through
                return std::forward<T>(arg);
            }

            template <typename T>
            static auto ConvertPortElementsArgImpl(Model& model, T&& arg, std::true_type, std::false_type)
            {
                // should not use initializer list
                return model.AddRoutingNodes(std::forward<T>(arg));
            }

            template <typename T>
            static auto ConvertPortElementsArgImpl(Model& model, T&& arg, std::true_type, std::true_type)
            {
                // should use initializer list
                return model.AddRoutingNodes({ std::forward<T>(arg) });
            }

            template <typename T>
            static decltype(auto) ConvertPortElementsArg(Model& model, T&& arg)
            {
                constexpr auto noPassThrough =
                    std::is_base_of<PortRange, std::decay_t<T>>{} ||
                    std::is_base_of<PortElementBase, std::decay_t<T>>{} ||
                    std::is_base_of<PortElementsBase, std::decay_t<T>>{};

                constexpr auto shouldUseInitList = !std::is_base_of<PortElementsBase, std::decay_t<T>>{};

                return ConvertPortElementsArgImpl(
                    model,
                    std::forward<T>(arg),
                    std::integral_constant<bool, noPassThrough>{},
                    std::integral_constant<bool, shouldUseInitList>{});
            }
        };
    }

    //
    // Factory method for creating nodes
    //
    template <typename NodeType, typename... Args>
    NodeType* Model::AddNode(Args&&... args)
    {
        auto node = std::make_unique<NodeType>(detail::ModelNodeRouter::ConvertPortElementsArg(*this, std::forward<Args>(args))...);
        auto result = node.get();
        AddExistingNode(std::move(node));
        return result;
    }

    template <typename ValueType>
    PortElements<ValueType> Model::AddRoutingNodes(const PortElements<ValueType>& elements)
    {
        return PortElements<ValueType>(AddRoutingNodes(static_cast<const PortElementsBase&>(elements)));
    }

    //
    // Compute output value
    //
    template <typename ValueType>
    std::vector<ValueType> Model::ComputeOutput(const OutputPort<ValueType>& outputPort) const
    {
        auto compute = [](const Node& node) { node.Compute(); };
        VisitSubset({ outputPort.GetNode() }, compute);
        return outputPort.GetOutput();
    }

    template <typename ValueType>
    std::vector<ValueType> Model::ComputeOutput(const PortElements<ValueType>& elements) const
    {
        // get set of nodes to make sure we visit
        std::unordered_set<const Node*> usedNodes;
        for (const auto& range : elements.GetRanges())
        {
            usedNodes.insert(range.ReferencedPort()->GetNode());
        }

        auto compute = [](const Node& node) { node.Compute(); };
        auto nodes = std::vector<const Node*>(usedNodes.begin(), usedNodes.end());
        VisitSubset(nodes, compute);

        // Now construct the output
        auto numElements = elements.Size();
        std::vector<ValueType> result(numElements);
        for (size_t index = 0; index < numElements; ++index)
        {
            auto element = elements.GetElement(index);
            auto port = element.ReferencedPort();
            auto portOutput = port->GetOutput()[element.GetIndex()];
            result[index] = portOutput;
        }
        return result;
    }

    template <typename ValueType>
    std::vector<ValueType> Model::ComputeOutput(const PortElementsBase& elements) const
    {
        auto typedElements = PortElements<ValueType>(elements);
        return ComputeOutput(typedElements);
    }

    //
    // Get nodes by type
    //
    template <typename NodeType>
    std::vector<const NodeType*> Model::GetNodesByType() const
    {
        std::vector<const NodeType*> result;
        auto findNodes = [&result](const Node& node) {
            auto nodePtr = dynamic_cast<const NodeType*>(&node);
            if (nodePtr != nullptr)
            {
                result.push_back(nodePtr);
            }
        };
        Visit(findNodes);
        return result;
    }

    template <typename NodeType>
    std::vector<NodeType*> Model::GetNodesByType()
    {
        std::vector<NodeType*> result;
        auto findNodes = [&result](const Node& node) {
            auto nodePtr = dynamic_cast<const NodeType*>(&node);
            if (nodePtr != nullptr)
            {
                result.push_back(const_cast<NodeType*>(nodePtr));
            }
        };
        Visit(findNodes);
        return result;
    }

    //
    // Visitors
    //

    // Visits the entire model
    template <typename Visitor>
    void Model::Visit(Visitor&& visitor) const
    {
        std::vector<const Node*> emptyVec;
        VisitSubset(emptyVec, visitor);
    }

    // Visits just the parts necessary to compute output node
    template <typename Visitor>
    void Model::VisitSubset(const Node* outputNode, Visitor&& visitor) const
    {
        VisitSubset(std::vector<const Node*>{ outputNode }, visitor);
    }

    // Real implementation function for `VisitSubset()`
    template <typename Visitor>
    void Model::VisitSubset(const std::vector<const Node*>& outputNodes, Visitor&& visitor) const
    {
        auto iter = GetNodeIterator(outputNodes);
        while (iter.IsValid())
        {
            visitor(*iter.Get());
            iter.Next();
        }
    }

    // Visits the entire model in reverse
    template <typename Visitor>
    void Model::ReverseVisit(Visitor&& visitor) const
    {
        auto iter = GetReverseNodeIterator();
        while (iter.IsValid())
        {
            visitor(*iter.Get());
            iter.Next();
        }
    }
}
}
