# Serialization

## Available Types

- Size Defined Integer Types: int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t. 
- Boolean Type
- std::array
- Enum Types: enum, enum class, enum struct
- Trivial Custom Classes

## Not Available Types

- Size Undefined Integer Types: char, int, long, etc. (Because some of types has different size in different environment. It may not occur errors but it may not works well)
- Float Types: float, double, etc. (Due to boost::endian library)
- Not Trivial Custom Classes
- Dynamic Containers: std::vector, std::map, etc.

## Usage

### Serialize

```cpp
int value;
auto data = serialize(value);
```

### Deserialize

```cpp
int value;
std::vector<uint8_t> data{...};
deserialize(value, data);
```

### Get Serialized Size

```cpp
int32_t value=0;
size_t size = get_serialized_size(value);
```

The size is always same if the type is same.

## Serialize Custom Type

To make custom type serializable, there are two ways as below.
The type must be a trivial type.

### Define on_serialize(serializer&) Member Function

```cpp
struct Data{
    int32_t value1;
    uint8_t value2;

    void on_serialize(serializer& serializer){
        serializer += value1;
        serializer += value2;
    }
};
```

### Define on_serialize(T& value, serializer&) Global Function in pgl Namespace

```cpp
struct Data{
    int32_t value1;
    uint8_t value2;
};

namespace pgl{
    void on_serialize(Data& data, serializer& serializer){
        serializer += data.value1;
        serializer += data.value2;
    }
}
```
