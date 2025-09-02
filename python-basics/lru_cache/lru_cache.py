import inspect
from collections import OrderedDict
from functools import wraps
from typing import Any, Callable

class _CacheStats:
    __slots__ = ('cache_hits', 'cache_misses')
    def __init__(self):
        self.cache_hits = 0
        self.cache_misses = 0

def lru_cache(max_items: int) -> Callable:
    """
    Функция создает декоратор, позволяющий кэшировать результаты выполнения обернутой функции по принципу LRU-кэша.
    Размер LRU кэша ограничен количеством max_items. При попытке сохранить новый результат в кэш, в том случае, когда
    размер кэша уже равен max_size, происходит удаление одного из старых элементов, сохраненных в кэше.
    Удаляется тот элемент, к которому обращались давнее всего.
    Также у обернутой функции должен появиться атрибут stats, в котором лежит объект с атрибутами cache_hits и
    cache_misses, подсчитывающие количество успешных и неуспешных использований кэша.
    :param max_items: максимальный размер кэша.
    :return: декоратор, добавляющий LRU-кэширование для обернутой функции.
    """

    if not isinstance(max_items, int) or max_items <= 0:
        raise ValueError(f"max_items must be a positive integer, got {max_items!r}")

    def decorator(func: Callable) -> Callable:
        sig = inspect.signature(func) # детерминированность + стабилизация kwargs и args 
        cache = OrderedDict()
        stats = _CacheStats()

        def _freeze(obj: Any) -> Any:
            from collections.abc import Hashable
            if isinstance(obj, Hashable) and not isinstance(obj, tuple):
                if not isinstance(obj, tuple):
                    return obj
            if isinstance(obj, tuple):
                return ('__tuple__', tuple(_freeze(e) for e in obj))
            if isinstance(obj, list):
                return ('__list__', tuple(_freeze(e) for e in obj))
            if isinstance(obj, dict):
                items = []
                for k, v in obj.items():
                    frozen_v = _freeze(v)
                    items.append((k, frozen_v)) # determa by sorting by key
                try:
                    items.sort(key=lambda x: x[0])
                except TypeError:
                    items.sort(key=lambda x: repr(x[0]))
                return ('__dict__', tuple(items))
            raise TypeError(f"Uncacheable type: {type(obj)}")

        @wraps(func)
        def wrapper(*args, **kwargs):
            try:
                bound = sig.bind_partial(*args, **kwargs)
                bound.apply_defaults()
            except Exception:
                stats.cache_misses += 1
                return func(*args, **kwargs)

            try:
                key_items = []
                for name, value in bound.arguments.items():
                    frozen = _freeze(value)
                    key_items.append((name, frozen))
                key = tuple(key_items)
                hash(key)
            except TypeError:
                stats.cache_misses += 1
                return func(*args, **kwargs)
            except Exception:
                stats.cache_misses += 1
                return func(*args, **kwargs)

            if key in cache:
                stats.cache_hits += 1
                cache.move_to_end(key, last=True)
                return cache[key]

            stats.cache_misses += 1
            result = func(*args, **kwargs)
            if len(cache) >= max_items:
                cache.popitem(last=False)
            cache[key] = result
            return result

        wrapper.stats = stats

        def cache_clear():
            cache.clear()
            stats.cache_hits = 0
            stats.cache_misses = 0
        wrapper.cache_clear = cache_clear

        def cache_info():
            return {
                'cache_hits': stats.cache_hits,
                'cache_misses': stats.cache_misses,
                'current_size': len(cache),
                'max_size': max_items,
            }
        wrapper.cache_info = cache_info

        return wrapper

    return decorator

