
from multiprocessing import Pool
from time import time
from itertools import repeat


class F:
    def __init__(self, y):
        self.y = y

    def __call__(self, x, *args, **kwargs):
        return x*x + self.y


if __name__ == '__main__':
    y = 3

    x_range = range(5000000)
    f = F(y)

    t1 = time()
    out_1 = [f(i) for i in x_range]
    t2 = time()
    print(t2-t1)

    t3 = time()


    with Pool(5) as p:
        out_2 = p.map(f, x_range)
    t4 = time()
    print(t4-t3)


