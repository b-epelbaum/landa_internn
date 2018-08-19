#pragma once
#include <string>

class IQBase
{

public:
	IQBase() = default;
	IQBase(const IQBase &) = delete;
	IQBase(IQBase &&) = delete;
	virtual ~IQBase() = default;

	const IQBase & operator = (const IQBase &) = delete;
	IQBase & operator = (IQBase &&) = delete;

	/**
	* \brief
	* This functions map the abstract interfaces to Qt QObject instances
	* \return
	*/
	virtual std::string	GetMetaClassFullName() const { return "Unknown"; }
	virtual std::string	GetMetaClassDebugName() const { return "Unknown"; }
};
