#pragma once

#include <memory>

#define DECLARE_DELEGATE_OneParam( DelegateName, Param1Type ) \
	FUNC_DECLARE_DELEGATE(DelegateName, void, Param1Type)

#define FUNC_DECLARE_DELEGATE( DelegateName, ReturnType, ...) \
	typedef Delegate<ReturnType(__VA_ARGS__)> DelegateName;


template<typename RetType, typename... Params>
class DelegateInstance
{
public:
	DelegateInstance() {};
	virtual ~DelegateInstance() {};

public:
	virtual RetType Execute(Params...) = 0;
};

template<typename Object, typename RetType, typename... Params>
class MemberFunctionInvoker : public DelegateInstance<RetType, Params...>
{
public:
	using Function = RetType(Object::*)(Params...);

	MemberFunctionInvoker(Object* object, Function function)
		:m_object(object),
		m_function(function)
	{
	};
	virtual ~MemberFunctionInvoker() {};

public:
	RetType Execute(Params...) override;

private:
	Object* m_object;
	Function m_function;
};

template <typename signature>
class Delegate;

template <typename RetType, typename... Params>
class Delegate<RetType(Params...)>{
public:
	
	Delegate() {};
	virtual ~Delegate() {};

public:
	template<typename Object>
	void Bind(Object* object, RetType(Object::* function)(Params...));

private:
	std::unique_ptr<DelegateInstance> m_instance;
};