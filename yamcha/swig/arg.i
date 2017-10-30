%include typemaps.i

/* java */ 
%typemap(java,jni)    (int argc, char *argv[]) "jobjectArray" 
%typemap(java,jtype)  (int argc, char *argv[]) "String[]"
%typemap(java,jstype) (int argc, char *argv[]) "String[]"

%typemap(java, in) (int argc, char *argv[]) (jstring *jsarray) {
  int i;

  $1 = JCALL1 (GetArrayLength, jenv, $input);
  if ($1 == 0) {
    SWIG_JavaThrowException(jenv, SWIG_JavaIndexOutOfBoundsException, "Array must contain at least 1 element");
    return $null;
  }
  $2 = new char * [$1+1];
  jsarray = new jstring [$1+1];
  for (i = 0; i < $1; i++) {
    jsarray[i] = (jstring) JCALL2 (GetObjectArrayElement, jenv, $input, i);
    $2[i] = (char *)       JCALL2 (GetStringUTFChars, jenv, jsarray[i], 0);
  }
  $2[i] = 0;
}

%typemap(argout) (int argc, char *argv[]) "" /* override char *[] default */

%typemap(java, freearg) (int argc, char *argv[]) {
  int i;
  for (i = 0; i < $1; i++) {
    JCALL2 (ReleaseStringUTFChars, jenv, jsarray$argnum[i], $2[i]);
  }
  delete [] $2;
}

/* perl */
%typemap(perl5,in) (int argc, char *argv[]) {
  AV *tempav;
  SV **tv;
  I32 len;
  int i;
  if (!SvROK($input)) {
    SWIG_exception(SWIG_ValueError,"$input is not an array.");
  }
  if (SvTYPE(SvRV($input)) != SVt_PVAV) {
    SWIG_exception(SWIG_ValueError,"$input is not an array.");
  }
  tempav = (AV*)SvRV($input);
  len = av_len(tempav);
  $1 = (int) len+1;
  
  $2 = new char * [$1 + 1];
  for (i = 0; i <= len; i++) {
    tv = av_fetch(tempav, i, 0);
    $2[i] = (char *) SvPV(*tv,PL_na);
  }
  $2[i] = 0;
}

%typemap(perl5,freearg) (int argc, char *argv[]) {
  delete [] $2;
}

/* python */ 
%typemap(python,in) (int argc, char *argv[]) {
  int i;
  if (!PyList_Check($input)) {
    SWIG_exception(SWIG_ValueError, "Expecting a list");
  }
  $1 = PyList_Size($input);
  if ($1 == 0) {
    SWIG_exception(SWIG_ValueError, "List must contain at least 1 element");
  }
 
  $2 = new char * [$1+1];
  for (i = 0; i < $1; i++) {
    PyObject *s = PyList_GetItem($input,i);
    if (!PyString_Check(s)) {
      SWIG_exception(SWIG_ValueError, "List items must be strings");
      delete [] $2;
      return NULL;
    }
    $2[i] = PyString_AsString(s);
  }
  $2[i] = 0;
}

%typemap(python,freearg) (int argc, char *argv[]) {
  delete [] $2;
}

/* ruby */
%typemap(ruby,in) (int argc, char *argv[]) {
  int i;
  
  if (TYPE($input) != T_ARRAY) {
    SWIG_exception(SWIG_ValueError, "Expected an array");
  }
  $1 = RARRAY($input)->len;
  if ($1 == 0) {
    SWIG_exception(SWIG_ValueError, "List must contain at least 1 element");
  }
  $2 = new char * [$1+1];
  for (i = 0; i < $1; i++) {
    VALUE   s = rb_ary_entry($input,i);
    if (TYPE(s) != T_STRING) {
      free($2);
      SWIG_exception(SWIG_ValueError, "List items must be strings");
    }
    $2[i] = STR2CSTR(s);
  }
  $2[i] = 0;
}

%typemap(ruby,freearg) (int argc, char *argv[]) {
  delete [] $2;
}
