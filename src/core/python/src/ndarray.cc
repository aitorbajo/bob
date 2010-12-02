/**
 * @author <a href="mailto:andre.anjos@idiap.ch">Andre Anjos</a>
 *
 * @brief Automatic converters for numpy.ndarray.
 */

#include "core/python/ndarray.h"
#include "core/python/TypeMapper.h"
#include <boost/python/refcount.hpp>
#include <boost/python/type_id.hpp>
#include <boost/format.hpp>

namespace bp = boost::python;
namespace tp = Torch::python;

template<typename T, int N> 
static bp::ndarray from_blitz(const blitz::Array<T,N>& b) {
  //if we get called, it is because we have one of the allowed types and the
  //number of dimensions was already checked.
  npy_intp dims[N];
  for (size_t i=0; i<N; ++i) dims[i] = b.extent(i);
  bp::ndarray npy = bp::new_ndarray(N, dims, tp::TYPEMAP.type_to_enum<T>());
  T* data = (T*)npy.data();
  size_t i = 0;
  for (typename blitz::Array<T,N>::const_iterator it=b.begin(); it!=b.end(); ++it, ++i) {
    data[i] = *it;
  }
  return npy;
}

#define NDARRAY_CTOR(BZ_ELEMENT_TYPE) \
template<> bp::ndarray::ndarray(const blitz::Array<BZ_ELEMENT_TYPE,1>& bz) : m_obj() { *this = from_blitz(bz); } \
template<> bp::ndarray::ndarray(const blitz::Array<BZ_ELEMENT_TYPE,2>& bz) : m_obj() { *this = from_blitz(bz); } \
template<> bp::ndarray::ndarray(const blitz::Array<BZ_ELEMENT_TYPE,3>& bz) : m_obj() { *this = from_blitz(bz); } \
template<> bp::ndarray::ndarray(const blitz::Array<BZ_ELEMENT_TYPE,4>& bz) : m_obj() { *this = from_blitz(bz); } 
NDARRAY_CTOR(bool)
NDARRAY_CTOR(int8_t)
NDARRAY_CTOR(uint8_t)
NDARRAY_CTOR(int16_t)
NDARRAY_CTOR(uint16_t)
NDARRAY_CTOR(int32_t)
NDARRAY_CTOR(uint32_t)
NDARRAY_CTOR(int64_t)
NDARRAY_CTOR(uint64_t)
NDARRAY_CTOR(float)
NDARRAY_CTOR(double)
NDARRAY_CTOR(long double)
NDARRAY_CTOR(std::complex<float>)
NDARRAY_CTOR(std::complex<double>)
NDARRAY_CTOR(std::complex<long double>)
#undef NDARRAY_CTOR

bp::ndarray::ndarray(const bp::object& obj)
  : m_obj(obj)
{
  check_obj(obj);
  bp::incref(obj.ptr());
}

bp::ndarray::ndarray(const bp::ndarray& obj)
  : m_obj(obj)
{
  //no need to check, of course
  bp::incref(obj.ptr());
}

bp::ndarray::~ndarray() {
  bp::decref(m_obj.ptr());
}

bp::ndarray& bp::ndarray::operator=
(const bp::ndarray& other) {
  bp::incref(other.m_obj.ptr()); //get other's
  bp::decref(m_obj.ptr()); //release self
  m_obj = other.m_obj;
  return *this;
}

void bp::ndarray::check_obj(const bp::object& obj) const {
  if(obj.ptr() != Py_None && !PyArray_Check(obj.ptr())) {
    PyErr_SetString(PyExc_TypeError, "Not a numpy.ndarray");
    throw bp::error_already_set();
  }
}

const bp::object& bp::ndarray::get_obj() const { 
  return m_obj; 
}

Py_ssize_t bp::ndarray::ndim() const { 
  return PyArray_NDIM(m_obj.ptr()); 
}

Py_ssize_t bp::ndarray::size() const { 
  return PyArray_SIZE(m_obj.ptr());
}

const npy_intp* bp::ndarray::shape() const { 
  return PyArray_DIMS(m_obj.ptr()); 
}

const npy_intp* bp::ndarray::strides() const { 
  return PyArray_STRIDES(m_obj.ptr()); 
}

Py_ssize_t bp::ndarray::itemsize() const { 
  return PyArray_ITEMSIZE(m_obj.ptr()); 
}

NPY_TYPES bp::ndarray::dtype() const { 
  return (NPY_TYPES)PyArray_TYPE(m_obj.ptr());
}

const void* bp::ndarray::data() const { 
  return PyArray_DATA(m_obj.ptr()); 
}

static inline bool can_cast(NPY_TYPES t1, NPY_TYPES t2) {
  return PyArray_CanCastSafely((int)t1, (int)t2);
}

static inline bool are_equivalent(NPY_TYPES t1, NPY_TYPES t2) {
  return PyArray_EquivTypenums((int)t1, (int)t2);
}

template <int N> static void assert_ndim(const bp::ndarray& self) {
  if (self.ndim() != N) {
    boost::format err("expected blitz::Array<T,%d>, got instead %d");
    err % self.ndim() % N;
    PyErr_SetString(PyExc_RuntimeError, err.str().c_str());
    boost::python::throw_error_already_set();
  }
}

template <typename T> static void assert_type(const bp::ndarray& self) {
  if (!can_cast(self.dtype(), tp::TYPEMAP.type_to_enum<T>())) {
    boost::format err("expected blitz::Array<%s,N> (or cast'able in the PyArray_CanCastSafely() sense - see the numpy C-API manual), got instead %s");
    err % tp::TYPEMAP.enum_to_name(self.dtype());
    err % tp::TYPEMAP.type_to_typename<T>();
    PyErr_SetString(PyExc_TypeError, err.str().c_str());
    boost::python::throw_error_already_set();
  }
}

/**
 * Converts a Numpy array to a blitz one copying the data and casting to
 * the destination element type. Please note that this is just a place
 * holder, only the full specializations are actual valid
 * implementations.
 */
template<typename T, int N> static boost::shared_ptr<blitz::Array<T,N> > 
convert_to_blitz (const bp::ndarray& self) {
  typedef blitz::Array<T,N> bz_type;
  typedef blitz::TinyVector<int,N> bz_shape;
  
  assert_ndim<N>(self);
  /**
   * Because we are dealing all the time with very simple types, we don't need
   * to make sure the types are castable between themselves, just do it if
   * needed.
   */
  //assert_type<T>(self);

  bz_shape shape;
  bz_shape stride;

  //if the type is exactly the same, no need to cast - this is by far
  //the most performant option.
  if (are_equivalent(self.dtype(), tp::TYPEMAP.type_to_enum<T>())) {
    const npy_intp* npy_shape = self.shape();
    const npy_intp* npy_stride = self.strides();
    for (npy_intp i=0;i<N;++i) {
      shape[i] = npy_shape[i];
      stride[i] = npy_stride[i] / sizeof(T);
    }
    return boost::shared_ptr<bz_type>(new bz_type((T*)self.data(), shape, stride, blitz::duplicateData));
  }

  //if you get to this point, there is no other option than casting
  //which may make you loose precision -- be warned ;-)
  bp::ndarray casted = self.astype(tp::TYPEMAP.type_to_enum<T>());
  const npy_intp* npy_shape = casted.shape();
  const npy_intp* npy_stride = casted.strides();
  for (npy_intp i=0;i<N;++i) {
    shape[i] = npy_shape[i];
    stride[i] = npy_stride[i] / sizeof(T);
  }
  return boost::shared_ptr<bz_type>(new bz_type((T*)casted.data(), shape, stride, blitz::duplicateData));
}

#define NDARRAY_TO_BLITZ(BZ_ELEMENT_TYPE) \
template<> boost::shared_ptr<blitz::Array<BZ_ELEMENT_TYPE,1> > bp::ndarray::to_blitz<BZ_ELEMENT_TYPE,1>() const { return convert_to_blitz<BZ_ELEMENT_TYPE,1>(*this); } \
template<> boost::shared_ptr<blitz::Array<BZ_ELEMENT_TYPE,2> > bp::ndarray::to_blitz<BZ_ELEMENT_TYPE,2>() const { return convert_to_blitz<BZ_ELEMENT_TYPE,2>(*this); } \
template<> boost::shared_ptr<blitz::Array<BZ_ELEMENT_TYPE,3> > bp::ndarray::to_blitz<BZ_ELEMENT_TYPE,3>() const { return convert_to_blitz<BZ_ELEMENT_TYPE,3>(*this); } \
template<> boost::shared_ptr<blitz::Array<BZ_ELEMENT_TYPE,4> > bp::ndarray::to_blitz<BZ_ELEMENT_TYPE,4>() const { return convert_to_blitz<BZ_ELEMENT_TYPE,4>(*this); } 
NDARRAY_TO_BLITZ(bool)
NDARRAY_TO_BLITZ(int8_t)
NDARRAY_TO_BLITZ(uint8_t)
NDARRAY_TO_BLITZ(int16_t)
NDARRAY_TO_BLITZ(uint16_t)
NDARRAY_TO_BLITZ(int32_t)
NDARRAY_TO_BLITZ(uint32_t)
NDARRAY_TO_BLITZ(int64_t)
NDARRAY_TO_BLITZ(uint64_t)
NDARRAY_TO_BLITZ(float)
NDARRAY_TO_BLITZ(double)
NDARRAY_TO_BLITZ(long double)
NDARRAY_TO_BLITZ(std::complex<float>)
NDARRAY_TO_BLITZ(std::complex<double>)
NDARRAY_TO_BLITZ(std::complex<long double>)
#undef NDARRAY_TO_BLITZ

static inline bp::object get_borrowed_object(PyObject* obj) {
  return bp::object(bp::handle<>(bp::borrowed(obj)));
}

static inline bp::object get_new_object(PyObject* obj) {
  return bp::object(bp::handle<>(obj));
}

bp::ndarray bp::ndarray::astype(NPY_TYPES dtype) const {
  return get_new_object(PyArray_Cast((PyArrayObject*)get_obj().ptr(), 
        (int)dtype));
}

bp::ndarray bp::new_ndarray(int len, npy_intp* shape, NPY_TYPES dtype) {
  return get_new_object(PyArray_SimpleNew(len, shape, (int)dtype));
}

struct ndarray_to_python {
  static PyObject* convert(const bp::ndarray& array) {
    return bp::incref(array.get_obj().ptr()); 
  }
  static const PyTypeObject* get_pytype() { return &PyArray_Type; }
};

//to and from python converters
void bind_ndarray () {
  import_array();

  struct from_python {
    static void* convertible(PyObject *py_obj) {
      return PyArray_Check(py_obj) ? py_obj : 0;
    }

    static void construct(PyObject *py_obj, 
        bp::converter::rvalue_from_python_stage1_data *data) {
      typedef bp::converter::rvalue_from_python_storage<bp::ndarray> storage_t;
      storage_t* the_storage = reinterpret_cast<storage_t*>(data);
      void* memory_chunk = the_storage->storage.bytes;
      new (memory_chunk) bp::ndarray(get_borrowed_object(py_obj));
      data->convertible = memory_chunk;
    }

    static const PyTypeObject* get_pytype() { return &PyArray_Type; }
  };

  bp::to_python_converter<bp::ndarray, ndarray_to_python
#if defined BOOST_PYTHON_SUPPORTS_PY_SIGNATURES
                          ,true
#endif
    >();

  bp::converter::registry::push_back(&from_python::convertible,
      &from_python::construct,
      bp::type_id<bp::ndarray>()
#if defined BOOST_PYTHON_SUPPORTS_PY_SIGNATURES
      , &from_python::get_pytype
#endif
      );
}
