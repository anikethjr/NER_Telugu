# This file was created automatically by SWIG.
# Don't modify this file, modify the SWIG interface instead.
# This file is compatible with both classic and new-style classes.

import _YamCha

def _swig_setattr(self,class_type,name,value):
    if (name == "this"):
        if isinstance(value, class_type):
            self.__dict__[name] = value.this
            if hasattr(value,"thisown"): self.__dict__["thisown"] = value.thisown
            del value.thisown
            return
    method = class_type.__swig_setmethods__.get(name,None)
    if method: return method(self,value)
    self.__dict__[name] = value

def _swig_getattr(self,class_type,name):
    method = class_type.__swig_getmethods__.get(name,None)
    if method: return method(self)
    raise AttributeError,name

import types
try:
    _object = types.ObjectType
    _newclass = 1
except AttributeError:
    class _object : pass
    _newclass = 0
del types


class Chunker(_object):
    __swig_setmethods__ = {}
    __setattr__ = lambda self, name, value: _swig_setattr(self, Chunker, name, value)
    __swig_getmethods__ = {}
    __getattr__ = lambda self, name: _swig_getattr(self, Chunker, name)
    def __repr__(self):
        return "<C SWIG_YamCha::Chunker instance at %s>" % (self.this,)
    def __init__(self, *args):
        _swig_setattr(self, Chunker, 'this', _YamCha.new_Chunker(*args))
        _swig_setattr(self, Chunker, 'thisown', 1)
    def __del__(self, destroy=_YamCha.delete_Chunker):
        try:
            if self.thisown: destroy(self)
        except: pass
    def parse(*args): return _YamCha.Chunker_parse(*args)
    def parseToString(*args): return _YamCha.Chunker_parseToString(*args)

class ChunkerPtr(Chunker):
    def __init__(self, this):
        _swig_setattr(self, Chunker, 'this', this)
        if not hasattr(self,"thisown"): _swig_setattr(self, Chunker, 'thisown', 0)
        _swig_setattr(self, Chunker,self.__class__,Chunker)
_YamCha.Chunker_swigregister(ChunkerPtr)


