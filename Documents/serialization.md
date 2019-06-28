# Serialization

## Available Types

- Integer Types: int, long, int32_t, etc.
- Float Types: float, double, etc.
- Boolean Type
- std::array
- Enum Types: enum, enum class, enum struct
- Trivial Custom Classes

## Not Available Types

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
int value=0;
size_t size = get_serialized_size(value);
```

The size is always same if the type is same.

## Serialize Custom Type

To make custom type serializable, there are two ways as below.
The type must be a trivial type.

### Define on_serialize(serializer&) Member Function

```cpp
struct Data{
    int value1;
    char value2;

    void on_serialize(serializer& serializer){
        serializer += value1;
        serializer += value2;
    }
};
```

### Define on_serialize(T& value, serializer&) Global Function in pgl Namespace

```cpp
struct Data{
    int value1;
    char value2;
};

namespace pgl{
    void on_serialize(Data& data, serializer& serializer){
        serializer += data.value1;
        serializer += data.value2;
    }
}
```
