////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Machine Learning Library (EMLL)
//  File:     IArchivable.tcc (utilities)
//  Authors:  Chuck Jacobs
//
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace emll
{
namespace utilities
{
    template <typename ValueType>
    static ValueType CreateObject(const ObjectArchive& archive)
    {
        return archive.GetValue<ValueType>();
    }
}
}
