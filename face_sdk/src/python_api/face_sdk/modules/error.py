class Error(Exception):

    def __init__(self, code: hex, what: str):
        self._code = code
        self._what = what

    def what(self):
        return self._what

    def code(self):
        if isinstance(self._code, int):
            return hex(self._code)
        else:
            return self._code

    def __str__(self):
        return "{}: {}".format(self.code(), self.what())
