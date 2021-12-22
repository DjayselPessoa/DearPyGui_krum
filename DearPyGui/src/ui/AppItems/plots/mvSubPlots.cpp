#include <algorithm>
#include "mvSubPlots.h"
#include "mvPlotLegend.h"
#include "mvContext.h"
#include "mvLog.h"
#include "mvAreaSeries.h"
#include "mvBarSeries.h"
#include "mvCandleSeries.h"
#include "mvErrorSeries.h"
#include "mvHeatSeries.h"
#include "mvImageSeries.h"
#include "mvInfiniteLineSeries.h"
#include "mvLabelSeries.h"
#include "mvPieSeries.h"
#include "mvScatterSeries.h"
#include "mvShadeSeries.h"
#include "mvStairSeries.h"
#include "mvStemSeries.h"
#include "mvPythonExceptions.h"
#include "mvPlotAxis.h"
#include "themes/mvTheme.h"
#include "containers/mvDragPayload.h"
#include "mvPyObject.h"

namespace Marvel {

	mvSubPlots::mvSubPlots(mvUUID uuid)
		: mvAppItem(uuid)
	{
		//_label = "SubPlot###" + std::to_string(_uuid);
		_width = -1;
		_height = -1;
	}

	void mvSubPlots::applySpecificTemplate(mvAppItem* item)
	{
		auto titem = static_cast<mvSubPlots*>(item);
		_rows = titem->_rows;
		_cols = titem->_cols;
		_row_ratios = titem->_row_ratios;
		_col_ratios = titem->_col_ratios;
		_flags = titem->_flags;
	}

	void mvSubPlots::onChildAdd(mvRef<mvAppItem> item)
	{
		if (item->_type == mvAppItemType::mvPlotLegend)
		{
			_flags &= ~ImPlotSubplotFlags_NoLegend;
			_flags |= ImPlotSubplotFlags_ShareItems;
		}
	}

	void mvSubPlots::onChildRemoved(mvRef<mvAppItem> item)
	{

		if (item->_type == mvAppItemType::mvPlotLegend)
		{
			_flags |= ImPlotSubplotFlags_NoLegend;
			_flags &= ~ImPlotSubplotFlags_ShareItems;
		}

	}

	void mvSubPlots::addFlag(ImPlotSubplotFlags flag)
	{
		_flags |= flag;
	}

	void mvSubPlots::removeFlag(ImPlotSubplotFlags flag)
	{
		_flags &= ~flag;
	}

	void mvSubPlots::draw(ImDrawList* drawlist, float x, float y)
	{

		ScopedID id(_uuid);

		if (ImPlot::BeginSubplots(_internalLabel.c_str(),  _rows, _cols, ImVec2((float)_width, (float)_height),
			_flags, _row_ratios.empty() ? nullptr : _row_ratios.data(), _col_ratios.empty() ? nullptr : _col_ratios.data()))
		{

			// plots
			for (auto& item : _children[1])
				item->draw(drawlist, x, y);

			ImPlot::EndSubplots();
		}
	}

	void mvSubPlots::handleSpecificRequiredArgs(PyObject* dict)
	{
		if (!VerifyRequiredArguments(GetParsers()[GetEntityCommand(_type)], dict))
			return;

		for (int i = 0; i < PyTuple_Size(dict); i++)
		{
			PyObject* item = PyTuple_GetItem(dict, i);
			switch (i)
			{
			case 0:
				_rows = ToInt(item);
				break;

			case 1:
				_cols = ToInt(item);
				break;

			default:
				break;
			}
		}
	}

	void mvSubPlots::handleSpecificKeywordArgs(PyObject* dict)
	{
		if (dict == nullptr)
			return;

		if (PyObject* item = PyDict_GetItemString(dict, "rows")) _rows = ToInt(item);
		if (PyObject* item = PyDict_GetItemString(dict, "columns")) _cols = ToInt(item);
		if (PyObject* item = PyDict_GetItemString(dict, "row_ratios")) _row_ratios = ToFloatVect(item);
		if (PyObject* item = PyDict_GetItemString(dict, "column_ratios")) _col_ratios = ToFloatVect(item);

		// helper for bit flipping
		auto flagop = [dict](const char* keyword, int flag, int& flags)
		{
			if (PyObject* item = PyDict_GetItemString(dict, keyword)) ToBool(item) ? flags |= flag : flags &= ~flag;
		};

		// subplot flags
		flagop("no_title", ImPlotSubplotFlags_NoTitle, _flags);
		flagop("no_menus", ImPlotSubplotFlags_NoMenus, _flags);
		flagop("no_resize", ImPlotSubplotFlags_NoResize, _flags);
		flagop("no_align", ImPlotSubplotFlags_NoAlign, _flags);
		flagop("link_rows", ImPlotSubplotFlags_LinkRows, _flags);
		flagop("link_columns", ImPlotSubplotFlags_LinkCols, _flags);
		flagop("link_all_x", ImPlotSubplotFlags_LinkAllX, _flags);
		flagop("link_all_y", ImPlotSubplotFlags_LinkAllY, _flags);
		flagop("column_major", ImPlotSubplotFlags_ColMajor, _flags);


	}

	void mvSubPlots::getSpecificConfiguration(PyObject* dict)
	{
		if (dict == nullptr)
			return;

		PyDict_SetItemString(dict, "rows", mvPyObject(ToPyInt(_rows)));
		PyDict_SetItemString(dict, "cols", mvPyObject(ToPyInt(_cols)));
		PyDict_SetItemString(dict, "row_ratios", mvPyObject(ToPyList(_row_ratios)));
		PyDict_SetItemString(dict, "column_ratios", mvPyObject(ToPyList(_col_ratios)));

		// helper to check and set bit
		auto checkbitset = [dict](const char* keyword, int flag, const int& flags)
		{
			PyDict_SetItemString(dict, keyword, mvPyObject(ToPyBool(flags & flag)));
		};

		// subplot flags
		checkbitset("no_title", ImPlotSubplotFlags_NoTitle, _flags);
		checkbitset("no_menus", ImPlotSubplotFlags_NoMenus, _flags);
		checkbitset("no_resize", ImPlotSubplotFlags_NoResize, _flags);
		checkbitset("no_align", ImPlotSubplotFlags_NoAlign, _flags);
		checkbitset("link_rows", ImPlotSubplotFlags_LinkRows, _flags);
		checkbitset("link_columns", ImPlotSubplotFlags_LinkCols, _flags);
		checkbitset("link_all_x", ImPlotSubplotFlags_LinkAllX, _flags);
		checkbitset("link_all_y", ImPlotSubplotFlags_LinkAllY, _flags);
		checkbitset("column_major", ImPlotSubplotFlags_ColMajor, _flags);
	}

}