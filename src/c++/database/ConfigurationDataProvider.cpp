#include "database.h"

const tissuestack::database::Configuration * tissuestack::database::ConfigurationDataProvider::queryConfigurationById(
		const std::string name) {
	if (name.empty())
		return nullptr;


	const std::string sql =
			"SELECT * FROM configuration WHERE name='"
			+ name + "';";

	tissuestack::database::Configuration * ret = nullptr;

	const pqxx::result results =
			tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(sql);

	if (results.size() == 0) return ret;
	if (results.size() > 1)
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
			"Unique key based search returned more than 1 record!");

	for (pqxx::result::const_iterator conf = results.begin(); conf != results.end(); ++conf)
	{
		ret = tissuestack::database::ConfigurationDataProvider::readResult(conf);
		break;
	}
	return ret;
}

const std::vector<const tissuestack::database::Configuration *> tissuestack::database::ConfigurationDataProvider::queryAllConfigurations()
{
	const std::string sql =
			"SELECT * FROM configuration;";

	std::vector<const tissuestack::database::Configuration *> ret;

	const pqxx::result results =
			tissuestack::database::TissueStackPostgresConnector::instance()->executeNonTransactionalQuery(sql);

	if (results.size() == 0) return ret;

	for (pqxx::result::const_iterator conf = results.begin(); conf != results.end(); ++conf)
		ret.push_back(tissuestack::database::ConfigurationDataProvider::readResult(conf));

	return ret;
}

inline tissuestack::database::Configuration * tissuestack::database::ConfigurationDataProvider::readResult(
		pqxx::result::const_iterator result)
{
	return 	new tissuestack::database::Configuration(
			result["name"].as<std::string>(),
			result["value"].as<std::string>(),
			result["description"].is_null() ? "" : result["description"].as<std::string>());
}