1
/**
 * @brief улучшенный аналог std::any, в отличии от стандартной any, знает, какой тип хранит. Цена - виртуальность + хранение имплементации по ссылке
 **/
class Any {
 public: 
  constexpr static Size InvalidTypeCode = 0;
 
  Any() {}
  Any( const Any & to_copy ) { set_implementation( to_copy.MakeHardCopy() ); }
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
  
 protected:
  template <typename Tn> class AnyImplementation;
  
  explicit Any( Any * implementation_ptr ) : implementation_( implementation_ptr ) {    
  }
  
  Any const *     implementation() const { return implementation_.get(); }
  Any       * get_implementation()       { return implementation_.get(); }
  void set_implementation( Any * new_implementation ) { implementation_.reset( new_implementation ); }
  
  virtual Any  * MakeHardCopy() const { return implementation() ? implementation()->MakeHardCopy() : nullptr; }
  virtual bool CompareWithOrDie( const Any & to_compare ) const { return implementation()->CompareWithOrDie( to_compare ); }
  template < typename Tn, typename... Args > static Tn * Allocate( Args&&... args ) { return new (std::nothrow) Tn( std::forward<Args>(args)... ); }
  //template < typename Tn, typename... Args > AnyImplementation<Tn> * AllocateImplementation( Args&&... args ) const { return Allocate< AnyImplementation<Tn> >( std::forward<Args>(args)... ); }
  
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
    This * re = AllocateStatic( object_ );
    //re->object_ = object_;
    return re; 
  }
  virtual bool CompareWithOrDie( const Any & to_compare ) const { return implementation()->CompareWithOrDie( to_compare ); }
  template < typename... Args > static This * AllocateStatic( Args&&... args ) { 
    return Any().Allocate< This > ( std::forward<Args>(args)... );
  }
 
 private:
  template< class T, class... Args > friend Any MakeAny( Args&&... args );
  friend void AnyDebug();
  
  Type object_;
};

//typedef

template< class Tn, class... Args >
Any MakeAny( Args&&... args ) {
  return Any( Any::AnyImplementation<Tn>::AllocateStatic( std::forward<Args>(args)... ) );
}
//template< class T, class U, class... Args >
//Any MakeAny( std::initializer_list<U> il, Args&&... args );
