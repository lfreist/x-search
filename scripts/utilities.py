import time
import math
import os


def timed(func):
    """
    decorator for getting a functions execution time
    :param func: function to be timed
    :return: float
    """
    def wrapper(*args, **kwargs) -> float:
        start = time.time()
        func(*args, **kwargs)
        end = time.time()
        return end - start
    return wrapper


def progress_bar(done: int, of: int, length: int = 100, percentage: bool = True, char: str = "|") -> str:
    """
    Return a progress bar string
    :param done:
    :param of:
    :param length:
    :param percentage:
    :param char:
    :return:
    """
    assert 0 < length <= 100, "Please provide a length in percentage (values 1-100)."
    # -2 because of '[' and ']'
    length = math.ceil(os.get_terminal_size().columns * (length/100))-2
    if length < 10:
        length = 10
    progress = done / of
    perc = ""
    if percentage:
        perc = str(round(progress * 100, 2)) + "%"
        # +1 because of the space
        length -= len(perc) + 1
    bars = char * math.ceil(progress * length)
    spaces = " " * (length - len(bars))
    return f"[{bars}{spaces} {perc}]" if percentage else f"[{bars}{spaces}]"
