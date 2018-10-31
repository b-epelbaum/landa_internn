#pragma once
#include <QDirIterator>
#include <QPluginLoader>
#include <QCoreApplication>
#include "interfaces/IQBAse.h"
#include "common/june_errors.h"
#include "common/june_exceptions.h"
#include "common/type_usings.h"


namespace LandaJune {
	namespace Parameters {
		class BaseParameters;
	}
}

namespace LandaJune {
	namespace Core {
		class FrameRef;
	}
}

namespace LandaJune
{
	namespace Algorithms
	{
		class IAlgorithmRunner : public IQBase
		{
		
		public:
			IAlgorithmRunner() = default;
			IAlgorithmRunner(const IAlgorithmRunner &) = delete;
			IAlgorithmRunner(IAlgorithmRunner &&) = delete;
			virtual ~IAlgorithmRunner() = default;
			const IAlgorithmRunner & operator = (const IAlgorithmRunner &) = delete;
			IAlgorithmRunner & operator = (IAlgorithmRunner &&) = delete;

			virtual std::unique_ptr<IAlgorithmRunner> clone() = 0;
			virtual QString getName() const = 0;
			virtual QString getDescription() const = 0;

			virtual bool isInited() const = 0;
			
			virtual void init(BaseParametersPtr parameters, Core::ICore* coreObject, CoreEventCallback callback ) = 0;
			virtual void cleanup() = 0;

			virtual CORE_ERROR process(const Core::FrameRef * frame) = 0;
			
			virtual std::shared_ptr<Parameters::BaseParameters> getParameters() const = 0;

			static std::list<std::shared_ptr<IAlgorithmRunner>> enumerateAlgorithmRunners();

		protected:

			static IAlgorithmRunner* loadNextAlgorithmRunner(const QString& strPath);
		};
	}
}

#define IAlgorithmRunner_iid "LandaCorp.com.JuneQCS.IAlgorithmRunner"
Q_DECLARE_INTERFACE(LandaJune::Algorithms::IAlgorithmRunner, IAlgorithmRunner_iid)
Q_DECLARE_METATYPE(std::shared_ptr<LandaJune::Algorithms::IAlgorithmRunner>)

using namespace LandaJune::Algorithms;

inline std::list<std::shared_ptr<IAlgorithmRunner>> IAlgorithmRunner::enumerateAlgorithmRunners()
{
	std::list<std::shared_ptr<IAlgorithmRunner>> retVal;
	const auto rootDir = qApp->applicationDirPath();
	QDirIterator it(rootDir, QStringList() << "algo_*.dll", QDir::Files);
	while (it.hasNext())
	{
		const auto rawPtr = loadNextAlgorithmRunner(it.next());
		if (rawPtr)
		{
			const auto& name = rawPtr->getName();
			std::shared_ptr<IAlgorithmRunner> ptr;
			ptr.reset(rawPtr, [](IAlgorithmRunner*) {});
			retVal.emplace_back(ptr);
		}
	}
	return retVal;
}

inline IAlgorithmRunner* IAlgorithmRunner::loadNextAlgorithmRunner(const QString& strPath)
{
	QPluginLoader loader(strPath);
	const auto plugin = loader.instance();
	if (plugin != nullptr)
	{
		return qobject_cast<IAlgorithmRunner*>(plugin);
	}
	THROW_EX_ERR_STR (CORE_ERROR::ERR_PROVIDER_DLL_CANNOT_BE_LOADED, loader.errorString().toStdString());
}