
namespace std {
template <typename _ostream, typename _Ty, template <typename, typename = allocator<_Ty>> typename _container>
_ostream & operator<<(_ostream & _out, _container<_Ty> rhs) {
  _out << "[";
  if (!rhs.empty()) {
    auto p = rhs.begin();
    _out << " " << *p;
    for (++p; p != rhs.end(); ++p) {
      _out << ", " << *p;
    }
  }
  _out << " ]";
  return _out;
}
} // namespace std
