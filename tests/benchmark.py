import pyrax
import random
import time

rax = pyrax.PyRax()

def key_gen(max_keys=1000000):
    for _ in range(max_keys):
        ken_len = random.randint(1, 20)
        # random_int = random.randint(48, 126)
        key = ''.join([chr(random.randint(48, 126)) for _ in range(ken_len)])
        yield key

def benchmark():
    rax = pyrax.PyRax()
    key_num = 10000000

    keys = set()
    st = time.time()
    for key in key_gen(key_num):
        keys.add(key)
    et = time.time()
    print('generate {} costs {}. average: {}'.format(key_num, et - st, (key_num / (et - st))))

    st = time.time()
    for key in keys:
        rax.insert(key, key)
    et = time.time()

    print('insert {} costs {}. average: {}'.format(key_num, et - st, (key_num / (et - st))))
    
    st = time.time()

    for key in keys:
        rax.find(key)

    et = time.time()

    print('find existed key {} costs {}. average: {}'.format(key_num, et - st, (key_num / (et - st))))
    st = time.time()

    for key in keys:
        rax.remove(key)
    
    et = time.time()
    print('remove key {} costs {}. average: {}'.format(key_num, et - st, (key_num / (et - st))))
    
    del rax
    
def main():
    for key in key_gen():
        rax.insert(key)

if __name__ == '__main__':
    benchmark()

