#pragma once

#include "Delegate.h"

template<typename Object, typename RetType, typename ...Params>
inline RetType MemberFunctionInvoker<Object, RetType, Params...>::Execute(Params... params)
{
	if (m_object == nullptr)
		return RetType();

	return (m_object->*m_function)(params...);
}

template<typename RetType, typename ...Params>
template<typename Object>
inline void Delegate<RetType(Params...)>::Bind(Object* object, RetType(Object::* function)(Params...))
{
	m_instance.reset(new MemberFunctionInvoker<Object, RetType, Params...>(object, function));
}