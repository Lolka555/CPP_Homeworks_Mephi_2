class EnhancedList(list):
    """
    Улучшенный list.
    Данный класс является наследником класса list и добавляет к нему несколько новых атрибутов.

    - first -- позволяет получать и задавать значение первого элемента списка.
    - last -- позволяет получать и задавать значение последнего элемента списка.
    - size -- позволяет получать и задавать длину списка:
        - если новая длина больше старой, то дополнить список значениями None;
        - если новая длина меньше старой, то удалять значения из конца списка.
    """

    @property
    def first(self):
        if not self:
            raise IndexError("List empty")
        return self[0]

    @first.setter
    def first(self, value):
        if not self:
            raise IndexError("List empty")
        self[0] = value

    @property
    def last(self):
        if not self:
            raise IndexError("List empty")
        return self[-1]

    @last.setter
    def last(self, value):
        if not self:
            raise IndexError("List empty")
        self[-1] = value

    @property
    def size(self):
        return len(self)

    @size.setter
    def size(self, new_size):
        if not isinstance(new_size, int) or new_size < 0:
            raise ValueError("Size < 0")
        
        if new_size > len(self):
            self.extend([None] * (new_size - len(self)))
        elif new_size < len(self):
            del self[new_size:]


