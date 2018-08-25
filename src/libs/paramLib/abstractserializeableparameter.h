#pragma once

#include <QObject>
#include <QJsonObject>

namespace LandaJune
{
	namespace Parameters
	{
		class AbstractSerializeableParameter : public QObject
		{
			Q_OBJECT

		public:
					AbstractSerializeableParameter() = default;
					AbstractSerializeableParameter(const QJsonObject& obj );
					AbstractSerializeableParameter(const AbstractSerializeableParameter& other) = default;
					~AbstractSerializeableParameter() = default;

					virtual std::string serialize();
					static bool deserialize(const QJsonObject& obj );

					signals:

					void propertyChanged(QString strPropName);

				private :

					using PROP_PAIR = std::pair<std::string, QVariant>;
					using PROP_LIST = std::vector<PROP_PAIR>;

					QJsonObject serialize(const PROP_LIST& propList) const;
		};
	}
}
