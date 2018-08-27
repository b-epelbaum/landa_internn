#pragma once
#include <QDirIterator>
#include <QPluginLoader>
#include <QCoreApplication>
#include "interfaces/IQBAse.h"


namespace LandaJune {
	namespace Parameters {
		class BaseParameter;
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
		class IAlgorithmHandler : public IQBase
		{
		
		public:
			IAlgorithmHandler() = default;
			IAlgorithmHandler(const IAlgorithmHandler &) = delete;
			IAlgorithmHandler(IAlgorithmHandler &&) = delete;
			virtual ~IAlgorithmHandler() = default;
			const IAlgorithmHandler & operator = (const IAlgorithmHandler &) = delete;
			IAlgorithmHandler & operator = (IAlgorithmHandler &&) = delete;

			virtual std::unique_ptr<IAlgorithmHandler> clone() = 0;
			virtual QString getName() const = 0;
			virtual QString getDescription() const = 0;
			
			virtual void init(std::shared_ptr<Parameters::BaseParameter> parameters) = 0;
			virtual void cleanup() = 0;
			
			virtual void process(const Core::FrameRef * frame) = 0;

			virtual std::shared_ptr<Parameters::BaseParameter> getParameters() const = 0;

			static std::list<std::shared_ptr<IAlgorithmHandler>> enumerateAlgorithmSets();

		protected:

			static IAlgorithmHandler* loadNextAlgorithmHandler(const QString& strPath);
		};
	}
}

#define IAlgorithmHandler_iid "LandaCorp.com.JuneQCS.IAlgorithmHandler"
Q_DECLARE_INTERFACE(LandaJune::Algorithms::IAlgorithmHandler, IAlgorithmHandler_iid)
Q_DECLARE_METATYPE(std::shared_ptr<LandaJune::Algorithms::IAlgorithmHandler>)

using namespace LandaJune::Algorithms;

inline std::list<std::shared_ptr<IAlgorithmHandler>> IAlgorithmHandler::enumerateAlgorithmSets()
{
	std::list<std::shared_ptr<IAlgorithmHandler>> retVal;
	const auto rootDir = qApp->applicationDirPath();
	QDirIterator it(rootDir, QStringList() << "algo_*.dll", QDir::Files);
	while (it.hasNext())
	{
		const auto rawPtr = loadNextAlgorithmHandler(it.next());
		if (rawPtr)
		{
			const auto& name = rawPtr->getName();
			std::shared_ptr<IAlgorithmHandler> ptr;
			ptr.reset(rawPtr, [](IAlgorithmHandler*) {});
			retVal.emplace_back(ptr);
		}
	}
	return retVal;
}

inline IAlgorithmHandler* IAlgorithmHandler::loadNextAlgorithmHandler(const QString& strPath)
{
	QPluginLoader loader(strPath);
	const auto plugin = loader.instance();
	if (plugin != nullptr)
	{
		return qobject_cast<IAlgorithmHandler*>(plugin);
	}
	return nullptr;
}