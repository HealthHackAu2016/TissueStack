/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "database.h"

tissuestack::database::DataSetInfo::DataSetInfo(
		const unsigned long long int id, const std::string filename) :
		_id(id), _filename(filename) {
	if (id < 0 || filename.empty() || filename.empty())
		THROW_TS_EXCEPTION(tissuestack::common::TissueStackApplicationException,
				"DataSetInfo Data Object needs a positive id and a non-empty filename!");
}

const unsigned long long int tissuestack::database::DataSetInfo::getId() const
{
	return this->_id;
}

const std::string tissuestack::database::DataSetInfo::getFileName() const
{
	return this->_filename;
}

const std::string tissuestack::database::DataSetInfo::getDescription() const
{
	return this->_description;
}

void tissuestack::database::DataSetInfo::setDescription(const std::string description)
{
	if (description.empty()) return;

	this->_description = description;
}

const std::string tissuestack::database::DataSetInfo::getZoomLevels() const
{
	return this->_zoom_levels;
}

void tissuestack::database::DataSetInfo::setZoomLevels(const std::string zoom_levels)
{
	if (zoom_levels.empty()) return;

	this->_zoom_levels = zoom_levels;
}

const bool tissuestack::database::DataSetInfo::isTiled() const
{
	return this->_is_tiled;
}

void tissuestack::database::DataSetInfo::setTiled(const bool is_tiled)
{
	this->_is_tiled = is_tiled;
}

void tissuestack::database::DataSetInfo::setOneToOneZoomLevel(const unsigned int one_to_one_zoom_level)
{
	if (one_to_one_zoom_level < 0) return;

	this->_one_to_one_zoom_level = one_to_one_zoom_level;
}

const unsigned int tissuestack::database::DataSetInfo::getOneToOneZoomLevel() const
{
	return this->_one_to_one_zoom_level;
}

void tissuestack::database::DataSetInfo::setResolutionMm(const float resolution_mm)
{
	if (resolution_mm < 0) return;

	this->_resolution_mm = resolution_mm;
}

const float tissuestack::database::DataSetInfo::getResolutionMm() const
{
	return this->_resolution_mm;
}


const std::string tissuestack::database::DataSetInfo::getJson(const bool everything) const
{
	std::ostringstream json;
	json << "{ \"id\": " << std::to_string(this->_id);
	json << ", \"filename\": \"" << tissuestack::utils::Misc::maskQuotesInJson(this->_filename) << "\"";
	if (!this->_description.empty())
		json << ", \"description\": \"" << tissuestack::utils::Misc::maskQuotesInJson(this->_description) << "\"";
	if (everything)
	{
		// TODO: add more
	}
	json << " }";

	return json.str();
}
