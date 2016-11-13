#pragma once

#include <memory>
#include <vector>
#include <functional>

class IterException : public std::exception
{

};

/// Iterator
template <typename T>
class Iterator
{
public:
	typedef std::function<T(const T&)> Mapper;
	typedef std::function<bool(const T&)> Predicate;
	typedef std::function<T(const T&, const T&)> Combiner;

    virtual ~Iterator();

	virtual T next() = 0;
};

template <typename T>
Iterator<T>::~Iterator()
{

}

template <typename T>
class ValuesIterator : public Iterator<T>
{
public:
	ValuesIterator(std::vector<T> items);

	virtual T next() override;

private:
	std::vector<T> items;
	size_t index = 0;
};

template <typename T>
ValuesIterator<T>::ValuesIterator(std::vector<T> items): items(items)
{

}
template <typename T>
T ValuesIterator<T>::next()
{
    if (this->index == this->items.size())
    {
        throw IterException();
    }

    return this->items[this->index++];
}


template <typename T, typename Mapped>
class MapIterator : public Iterator<Mapped>
{
public:
	MapIterator(std::function<Mapped(const T&)> mapper, std::unique_ptr<Iterator<T>> parent);

	virtual Mapped next() override;

private:
	std::function<Mapped(const T&)> mapper;
	std::unique_ptr<Iterator<T>> parent;
};

template <typename T, typename Mapped>
MapIterator<T, Mapped>::MapIterator(std::function<Mapped(const T&)> mapper, std::unique_ptr<Iterator<T>> parent): mapper(mapper), parent(std::move(parent))
{

}
template <typename T, typename Mapped>
Mapped MapIterator<T, Mapped>::next()
{
    return this->mapper(this->parent->next());
}


template <typename T>
class FilterIterator : public Iterator<T>
{
public:
	FilterIterator(typename Iterator<T>::Predicate predicate, std::unique_ptr<Iterator<T>> parent);

	virtual T next() override;

private:
	typename Iterator<T>::Mapper predicate;
	std::unique_ptr<Iterator<T>> parent;
};

template <typename T>
FilterIterator<T>::FilterIterator(typename Iterator<T>::Predicate predicate, std::unique_ptr<Iterator<T>> parent)
        : predicate(predicate), parent(std::move(parent))
{

}
template <typename T>
T FilterIterator<T>::next()
{
    while (true)
    {
        T item = this->parent->next();
        if (this->predicate(item))
        {
            return item;
        }
    }
}


template <typename T>
class TakeIterator : public Iterator<T>
{
public:
	TakeIterator(size_t count, std::unique_ptr<Iterator<T>> parent);

	virtual T next() override;

private:
	size_t count;
	size_t index = 0;
	std::unique_ptr<Iterator<T>> parent;
};

template <typename T>
TakeIterator<T>::TakeIterator(size_t count, std::unique_ptr<Iterator<T>> parent): count(count), parent(std::move(parent))
{

}
template <typename T>
T TakeIterator<T>::next()
{
    if (this->index < this->count)
    {
        this->index++;
        return this->parent->next();
    }
    else throw IterException();
}


/// Stream
template <typename T>
class Stream
{
public:
	typedef std::function<T(const T&)> Mapper;
	typedef std::function<bool(const T&)> Predicate;
	typedef std::function<T(const T&, const T&)> Combiner;

	virtual ~Stream();

	template <typename Mapped>
	Stream<Mapped>* map(std::function<Mapped(const T&)> mapper);
	Stream<T>* filter(Predicate predicate);
	Stream<T>* take(size_t count);
	T reduce(T init, Combiner combiner);

	std::vector<T> collect();

	virtual std::unique_ptr<Iterator<T>> iterate() = 0;
};
template <typename T>
class ValueStream : public Stream<T>
{
public:
    ValueStream(std::vector<T> items);

    std::unique_ptr<Iterator<T>> iterate() override;

private:
    std::vector<T> items;
};

template <typename T, typename Par = T>
class CompositedStream : public Stream<T>
{
public:
    CompositedStream(Stream<Par>* parent);
    virtual ~CompositedStream();

protected:
    Stream<Par>* parent;
};

template <typename T, typename Mapped>
class Map : public CompositedStream<Mapped, T>
{
public:
    Map(std::function<Mapped(const T&)> mapper, Stream<T>* parent);

    std::unique_ptr<Iterator<Mapped>> iterate() override;

private:
    std::function<Mapped(const T&)> mapper;
};

template <typename T>
class Filter : public CompositedStream<T>
{
public:
    Filter(typename Stream<T>::Predicate predicate, Stream<T>* parent);

    std::unique_ptr<Iterator<T>> iterate() override;

private:
    typename Stream<T>::Predicate predicate;
};

template <typename T>
class Take : public CompositedStream<T>
{
public:
    Take(size_t count, Stream<T>* parent);

    std::unique_ptr<Iterator<T>> iterate() override;

private:
    size_t count;
};

/// Stream
template <typename T>
Stream<T>::~Stream()
{

}
template<typename T>
template<typename Mapped>
Stream<Mapped>* Stream<T>::map(std::function<Mapped(const T&)> mapper)
{
    return new Map<T, Mapped>(mapper, this);
}
template <typename T>
Stream<T>* Stream<T>::filter(Stream<T>::Predicate predicate)
{
    return new Filter<T>(predicate, this);
}
template <typename T>
Stream<T>* Stream<T>::take(size_t count)
{
    return new Take<T>(count, this);
}
template <typename T>
T Stream<T>::reduce(T init, Stream<T>::Combiner combiner)
{
    std::unique_ptr<Iterator<T>> iterator = this->iterate();
    try
    {
        while (true)
        {
            init = combiner(init, iterator->next());
        }
    }
    catch (IterException e)
    {

    }

    return init;
}
template <typename T>
std::vector<T> Stream<T>::collect()
{
    std::unique_ptr<Iterator<T>> iter = this->iterate();
    std::vector<T> items;

    try
    {
        while (true)
        {
            items.push_back(iter->next());
        }
    }
    catch (IterException e)
    {

    }

    return items;
}

/// ValueStream
template <typename T>
ValueStream<T>::ValueStream(std::vector<T> items): items(items)
{

}
template <typename T>
std::unique_ptr<Iterator<T>> ValueStream<T>::iterate()
{
    return std::make_unique<ValuesIterator<T>>(this->items);
}

/// CompositeStream
template <typename T, typename Par>
CompositedStream<T, Par>::CompositedStream(Stream<Par>* parent): parent(parent)
{

}

template <typename T, typename Par>
CompositedStream<T, Par>::~CompositedStream()
{
    delete this->parent;
}

/// Map
template <typename T, typename Mapped>
Map<T, Mapped>::Map(std::function<Mapped(const T&)> mapper, Stream<T>* parent)
        : CompositedStream<Mapped, T>(parent), mapper(mapper)
{

}
template <typename T, typename Mapped>
std::unique_ptr<Iterator<Mapped>> Map<T, Mapped>::iterate()
{
    return std::make_unique<MapIterator<T, Mapped>>(this->mapper, this->parent->iterate());
}

/// Filter
template <typename T>
std::unique_ptr<Iterator<T>> Filter<T>::iterate()
{
    return std::make_unique<FilterIterator<T>>(this->predicate, this->parent->iterate());
}
template <typename T>
Filter<T>::Filter(typename Stream<T>::Predicate predicate, Stream<T>* parent)
        : CompositedStream<T>(parent), predicate(predicate)
{

}

/// Take
template <typename T>
Take<T>::Take(size_t count, Stream<T>* parent)
        : CompositedStream<T>(parent), count(count)
{

}
template <typename T>
std::unique_ptr<Iterator<T>> Take<T>::iterate()
{
    return std::make_unique<TakeIterator<T>>(this->count, this->parent->iterate());
}
