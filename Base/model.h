#ifndef MODEL_H
#define MODEL_H

#include <algorithm>
#include <map>
#include <optional>
#include <set>

using namespace std;

#include "common.h"
#include "event.h"

namespace Base::Model {

/**
 * TODO Document me!
 */
template <typename T> class Property : private Base::NonCopyable {
  public:
    /**
     * @brief Creates a new Property without a value.
     */
    explicit Property() {}

    /**
     * @brief Creates a new Property with the given value.
     *
     * @param value the initial value of the property.
     */
    explicit Property(const T& value) : _value(value) {}

    /**
     * @brief Checks if this Property is empty.
     *
     * @return true if the property is empty, false if it has a value.
     */
    bool isEmpty() const { return !_value.has_value(); }

    /**
     * @brief Checks if this Property has a value.
     *
     * @return true if the property has a value, false if it is empty.
     */
    bool hasValue() const { return _value.has_value(); }

    /**
     * @brief Sets the value of this Property to the given value. You can also
     * use the assignment operator.
     *
     * @param value the new value to set.
     */
    void setValue(const T& value) {
        _value = value;
        _valueChanged.fire(*this, value);
    }

    Property<T>& operator=(const T& value) {
        setValue(value);
        return *this;
    }

    bool operator==(const T& value) const {
        return _value.has_value() && _value.value() == value;
    }

    bool operator!=(const T& value) const {
        return !_value.has_value() || _value.value() != value;
    }

    bool operator<(const T& value) const {
        return !_value.has_value() || _value.value() < value;
    }

    bool operator>(const T& value) const {
        return _value.has_value() && _value.value() > value;
    }

    /**
     * @brief Returns the value of this Property. If the property is empty, an
     * exception is thrown.
     *
     * @return the current value.
     */
    T const& value() const { return _value.value(); }

    /**
     * @brief Clears this Property.
     */
    void clear() {
        _value.reset();
        _cleared.fire(*this);
    }

    EVENT(valueChanged, Property<T>&, T)
    EVENT(cleared, Property<T>&)

  private:
    optional<T> _value;
};

template <typename T>
inline bool operator==(const Property<T>& p1, const Property<T>& p2) {
    return (p1.isEmpty() && p2.isEmpty()) ||
           (p1.hasValue() && p2.hasValue() && p1.value() == p2.value());
}

template <typename T>
inline bool operator!=(const Property<T>& p1, const Property<T>& p2) {
    return !(p1 == p2);
}

template <typename T>
inline bool operator<(const Property<T>& p1, const Property<T>& p2) {
    return (p1.isEmpty() && p2.hasValue()) ||
           (p1.hasValue() && p2.hasValue() && p1.value() < p2.value());
}

template <typename T>
inline bool operator>(const Property<T>& p1, const Property<T>& p2) {
    return (p1.hasValue() && p2.isEmpty()) ||
           (p1.hasValue() && p2.hasValue() && p1.value() > p2.value());
}

template <typename Id> class SortView {
    using size_type = typename std::vector<Id>::size_type;

  public:
    explicit SortView() : _sortedIds(vector<Id>(0)) {}
    explicit SortView(const vector<Id>& sortedIds) : _sortedIds(sortedIds) {}

    size_type size() const { return _sortedIds.size(); }

    Id at(const size_type index) const { return _sortedIds.at(index); }
    Id at(const int index) const { return at(index); }

  private:
    vector<Id> _sortedIds;
};

/**
 * TODO document me
 */
template <typename Id, typename Item>
class Collection : private Base::NonCopyable {
    using CompareFunction = bool (*)(Item const&, Item const&);
    using SmartItemPointer = unique_ptr<Item>;
    using size_type = typename std::map<Id, SmartItemPointer>::size_type;

  public:
    /**
     * @brief Collection
     * @param idFunction
     */
    explicit Collection(const function<Id(Item const&)>& idFunction)
        : _idFunction(idFunction) {}

    /**
     * @brief Checks if this collection is empty.
     * @return true if this collection is empty, false if it contains at least
     * one item.
     */
    bool isEmpty() const { return _items.empty(); }

    /**
     * @brief Checks if this collection has any items.
     * @return true if this collection has at least one item, false if it is
     * empty.
     */
    bool hasItems() const { return !_items.empty(); }

    /**
     * @brief Returns the size of this collection (i.e. the number of items in
     * it).
     * @return the collection size.
     */
    size_type size() const { return _items.size(); }

    /**
     * @brief contains
     * @param id
     * @return
     */
    bool contains(const Id& id) const { return _items.count(id) > 0; }

    /**
     * @brief ids
     * @return
     */
    set<Id> const& ids() const { return _ids; }

    /**
     * @brief findById
     * @param id
     * @return
     */
    Item& findById(const Id& id) const { return *_items.at(id); }

    /**
     * @brief add
     * @param item
     */
    void add(Item* item) {
        auto id = _idFunction(*item);
        if (!contains(id)) {
            _ids.insert(id);
            _items[id] = SmartItemPointer(item);
            _itemAdded.fire(*this, id, *item);
        }
    }

    /**
     * @brief add
     * @param item
     */
    Item* addCopy(const Item& item) {
        auto copy = new Item(item);
        add(copy);
        return copy;
    }

    /**
     * @brief removeById
     * @param id
     */
    void removeById(const Id& id) {
        if (_items.erase(id) > 0) {
            _ids.erase(id);
            _itemRemoved.fire(*this, id);
        }
    }

    /**
     * @brief remove
     * @param item
     */
    void remove(const Item& item) { removeById(getItemId(item)); }

    /**
     * @brief clear
     */
    void clear() {
        _items.clear();
        _cleared.fire(*this);
    }

    SortView<Id> sort(const CompareFunction& compareFunction) const {
        if (_items.size() > 0) {
            vector<SmartItemPointer> sortVector(_items.size());
            for (const auto& kv : _items) {
                sortVector.push_back(kv.second);
            }
            sort(sortVector.begin(), sortVector.end(),
                 [compareFunction](SmartItemPointer const& i1,
                                   SmartItemPointer const& i2) {
                     return compareFunction(*i1, *i2);
                 });
            vector<Id> sortedIds(sortVector.size());
            for (const auto& item : sortVector) {
                sortedIds.push_back(item->id());
            }
            return SortView<Id>(sortedIds);
        } else {
            return SortView<Id>();
        }
    }

    ~Collection() {}

    EVENT(itemAdded, Collection<Id, Item>&, Id, Item&)
    EVENT(itemRemoved, Collection<Id, Item>&, Id)
    EVENT(cleared, Collection<Id, Item>&)

  private:
    function<Id(Item const&)> _idFunction;
    map<Id, SmartItemPointer> _items;
    set<Id> _ids;
    vector<Id> _sortedIds;
};

template <typename Id> class Identifiable {
  public:
    Identifiable(const Id& id) : _id(id) {}

    Id id() const { return _id; }

  private:
    Id _id;
};

} // namespace Base::Model

// TODO Document this macro in some way
#define PROPERTY(type, name)                                                   \
  private:                                                                     \
    Base::Model::Property<type> _##name;                                       \
                                                                               \
  public:                                                                      \
    Base::Model::Property<type>& name() { return _##name; }                    \
    Base::Model::Property<type> const& name() const { return _##name; }

#endif // MODEL_H
