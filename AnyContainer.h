//#include <utility>
#include <cstdio>
#include <new>
#include <memory>
#include <cassert>

typedef size_t Size;
template <typename Tn> using UniquePointer = std::unique_ptr<Tn>;

#define ANY_CONTAINER_UNIVERSAL_ASSERT( Condition, Message, ThrowObject) if ( ! (Condition) ) { assert(0); }
#define ANY_CONTAINER_CAST_ERROR_MESSAGE_TEXT "AnyCastError"

class Any;

namespace std {
template <typename ValueType> ValueType any_cast( const Any&  operand );
template <typename ValueType> ValueType any_cast(       Any&  operand );
template <typename ValueType> ValueType any_cast(       Any&& operand );
template <typename ValueType> ValueType any_cast( const Any*  operand );
template <typename ValueType> ValueType any_cast(       Any*  operand );
} // namespace std

/**
 * @brief улучшенный аналог std::any, в отличии от стандартной any, знает, какой тип хранит. Цена - виртуальность + хранение имплементации по ссылке
 **/
class Any {
 public: 
  constexpr static Size InvalidTypeCode = 0;
 
  Any() {}
  Any( const Any & to_copy ) { set_implementation( to_copy.MakeHardCopy() ); }
  Any( Any && to_move ) { set_implementation( to_move.ReleaseImplementation() );  }
  virtual ~Any() {}    
  Any & operator=( const Any & to_copy ) { 
    set_implementation( to_copy.MakeHardCopy() );
    return *this;
  }
  
  bool operator==( const Any & to_compare ) {
    if ( implementation() == to_compare.implementation() ) return true;
    if (   ( !implementation() || !to_compare.implementation() )   ) return false;
    if ( GetTypeCode() != to_compare.GetTypeCode() ) return false;
    return CompareWithOrDie( to_compare );
  }
  
  virtual Size GetTypeCode() const{ return implementation() ? implementation()->GetTypeCode() : InvalidTypeCode; }
  template <typename Tn> static Size GetCodeOf() { return typeid(Tn).hash_code; }
  
 protected:
  template <typename Tn> class AnyImplementation;
  
  Any( Any * implementation_ptr ) : implementation_( implementation_ptr ) {    
  }
  
  Any const *     implementation() const { return implementation_.get(); }
  Any       * get_implementation()       { return implementation_.get(); }
  void set_implementation( Any * new_implementation ) { implementation_.reset( new_implementation ); }
  
  virtual Any  * MakeHardCopy() const { return implementation() ? implementation()->MakeHardCopy() : nullptr; }
  virtual bool CompareWithOrDie( const Any & to_compare ) const { return implementation()->CompareWithOrDie( to_compare ); }
  template < typename Tn, typename... Args > Tn * Allocate( Args&&... args ) const { return new (std::nothrow) Tn( std::forward<Args>(args)... ); }
  template < typename Tn, typename... Args > AnyImplementation<Tn> * AllocateImplementation( Args&&... args ) const { return Allocate< AnyImplementation<Tn> >( std::forward<Args>(args)... ); }
  Any * ReleaseImplementation() { return implementation_.release(); }
  
 private:
  template< class T, class... Args > friend Any MakeAny( Args&&... args );
  friend void AnyDebug();
  
  UniquePointer<Any>  implementation_;
};

template <typename Tn> class Any::AnyImplementation : public Any {
 public:
  typedef Tn Type;
  typedef AnyImplementation<Type> This;
  
  template < typename... Args > explicit AnyImplementation( Args&&... args ) : object_( std::forward<Args>(args)... ) {
  }
  virtual ~AnyImplementation() {}
  
 protected:  
  virtual Any  * MakeHardCopy() const { 
    This * re = Any::Allocate< This >();
    re->object_ = object_;
    return re; 
  }
  virtual bool CompareWithOrDie( const Any & to_compare ) const { return implementation()->CompareWithOrDie( to_compare ); }
  template < typename... Args > static This * AllocateStatic( Args&&... args ) { return new (std::nothrow) This( std::forward<Args>(args)... ); }
 
 private:
  template< class T, class... Args > friend Any MakeAny( Args&&... args );
  friend void AnyDebug();
  
  template <typename ValueType> friend ValueType std::any_cast( const Any&  operand );
  template <typename ValueType> friend ValueType std::any_cast(       Any&  operand );
  template <typename ValueType> friend ValueType std::any_cast(       Any&& operand );
  template <typename ValueType> friend ValueType std::any_cast( const Any*  operand );
  template <typename ValueType> friend ValueType std::any_cast(       Any*  operand );
  
  Type object_;
};

//typedef

template< class Tn, class... Args >
Any MakeAny( Args&&... args ) {
  return Any( Any::AnyImplementation<Tn>::AllocateStatic( std::forward<Args>(args)... ) );
}
//template< class T, class U, class... Args >
//Any MakeAny( std::initializer_list<U> il, Args&&... args );


namespace std {

template <typename ValueType>
ValueType any_cast(const Any& operand) {
  ANY_CONTAINER_UNIVERSAL_ASSERT( operand.GetTypeCode() == Any::GetCodeOf< ValueType >, ANY_CONTAINER_CAST_ERROR_MESSAGE_TEXT "_1" , AnyCastError );
  typedef Any::AnyImplementation<ValueType> AnyImpl;
  return dynamic_cast<const AnyImpl *>( operand.implementation() ).object_;
}

template <typename ValueType>
ValueType any_cast(Any& operand) {
  ANY_CONTAINER_UNIVERSAL_ASSERT( operand.GetTypeCode() == Any::GetCodeOf< ValueType >, ANY_CONTAINER_CAST_ERROR_MESSAGE_TEXT "_2" , AnyCastError );
  typedef Any::AnyImplementation<ValueType> AnyImpl;
  return dynamic_cast<const AnyImpl *>( operand.implementation() ).object_;
}

template <typename ValueType>
ValueType any_cast(Any&& operand) {
  ANY_CONTAINER_UNIVERSAL_ASSERT( operand.GetTypeCode() == Any::GetCodeOf< ValueType >, ANY_CONTAINER_CAST_ERROR_MESSAGE_TEXT "_3" , AnyCastError );
  typedef Any::AnyImplementation<ValueType> AnyImpl;
  return dynamic_cast<const AnyImpl *>( operand.implementation() ).object_;
}

template <typename ValueType>
const ValueType* any_cast(const Any* operand) noexcept {
  ANY_CONTAINER_UNIVERSAL_ASSERT( operand->GetTypeCode() == Any::GetCodeOf< ValueType >, ANY_CONTAINER_CAST_ERROR_MESSAGE_TEXT "_4" , AnyCastError );
  typedef Any::AnyImplementation<ValueType> AnyImpl;
  return dynamic_cast<const AnyImpl *>( operand->implementation() ).object_;
}

template <typename ValueType>
ValueType* any_cast(Any* operand) noexcept {    
  ANY_CONTAINER_UNIVERSAL_ASSERT( operand->GetTypeCode() == Any::GetCodeOf< ValueType >, ANY_CONTAINER_CAST_ERROR_MESSAGE_TEXT "_5" , AnyCastError );
  typedef Any::AnyImplementation<ValueType> AnyImpl;
  return dynamic_cast<const AnyImpl *>( operand->implementation() ).object_;
}

} // namespace std




void AnyDebug() {
  Any::AnyImplementation<int> i;
  auto * ptr = Any::AnyImplementation<int>::AllocateStatic();
  Any x( (Any*)ptr);
}
