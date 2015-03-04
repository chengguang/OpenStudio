/**********************************************************************
*  Copyright (c) 2008-2015, Alliance for Sustainable Energy.  
*  All rights reserved.
*  
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*  
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*  
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**********************************************************************/

#include "FloodPlot.hpp"
#include "../time/Time.hpp"
//#include "../core/Application.hpp"

using namespace std;
using namespace boost;

namespace openstudio{

FloodPlotColorMap::FloodPlotColorMap(const Vector& colorLevels, FloodPlotColorMap::ColorMapList colorMapList)
  : QwtLinearColorMap(),
    m_colorLevels(colorLevels),
    m_colorMapList(colorMapList)
{
  setMode(QwtLinearColorMap::FixedColors); // no interpolation - can set in constructor
  init();
}

FloodPlotColorMap::~FloodPlotColorMap()
{
}

void FloodPlotColorMap::init()
{
  /// set color stops
  /// scale between 0 and 1
  unsigned int colormapLength = m_colorLevels.size();  // start and end colors
  int r, g, b;
  unsigned int i,j;
  QColor minColor, maxColor; // colors at interval ends
  double min = openstudio::minimum(m_colorLevels);
  double max = openstudio::maximum(m_colorLevels);
  if (max == min)
  {
    return;
  }
  switch (m_colorMapList)
  {
    case Gray:
      minColor = QColor(0,0,0);
      maxColor = QColor(255,255,255);
      setColorInterval(minColor, maxColor); // end points
            for (i = 0; i < colormapLength; i++)
            {
                double gray = 1.0 * i / (colormapLength - 1);
                r = (int)(255 * gray);
                g = (int)(255 * gray);
                b = (int)(255 * gray);
        addColorStop( (m_colorLevels(i) - min) / (max - min), QColor(r, g, b));
            }
      break;
    case Jet:
      Matrix cMatrix(colormapLength,3);
            unsigned int n = (int)ceil(colormapLength / 4.0);
            int nMod = 0;
            Vector  fArray(3 * n - 1);
      Vector  red(fArray.size());
            Vector  green(fArray.size());
            Vector  blue(fArray.size());


      for (i = 0; i < colormapLength; i++)
            {
        cMatrix(i, 0) = 0;
        cMatrix(i, 1) = 0;
        cMatrix(i, 2) = 0;
            }


            if (colormapLength % 4 == 1)
            {
                nMod = 1;
            }

      for (i = 0; i <fArray.size(); i++)
            {
                if (i < n)
                    fArray(i) = (float)(i + 1) / n;
                else if (i >= n && i < 2 * n - 1)
                    fArray(i) = 1.0;
                else if (i >= 2 * n - 1)
                    fArray(i) = (float)(3 * n - 1 - i) / n;
        green(i) = (int)ceil(n / 2.0) - nMod + i;
                red(i) = green(i) + n;
                blue(i) = green(i) - n;
            }

            int nb = 0;
            for (i = 0; i < blue.size(); i++)
            {
                if (blue(i) > 0)
                    nb++;
            }

            for (i = 0; i < colormapLength; i++)
            {
                for (j = 0; j < red.size(); j++)
                {
                    if (i == red(j) && red(j) < colormapLength)
                    {
                        cMatrix(i, 0) = fArray(i - red(0));
                    }
                }
                for (j = 0; j < green.size(); j++)
                {
                    if (i == green(j) && green(j) < colormapLength)
                        cMatrix(i, 1) = fArray(i - (int)green(0));
                }
                for ( j = 0; j < blue.size(); j++)
                {
                    if (i == blue(j) && blue(j) >= 0)
            cMatrix(i, 2) = fArray(fArray.size() - 1 - nb + i);
                }
            }

      // set before adding color stops
      // default from http://www.matthiaspospiech.de/blog/2008/06/16/qwt-spectrogramm-plot-with-data-arrays/
      minColor = QColor(0,0,189);
      maxColor = QColor(132,0,0);
      int bMin=256;
      int rMin=256;
            for (i = 0; i < colormapLength; i++)
            {
        r = (int)(cMatrix(i, 0) * 255);
        g = (int)(cMatrix(i, 1) * 255);
        b = (int)(cMatrix(i, 2) * 255);
        if ((r==0) && (g==0) && (b<bMin))
        {
          bMin=b;
          minColor = QColor(r,g,b);
        }
        if ((b==0) && (g==0) && (r<rMin))
        {
          rMin=r;
          maxColor = QColor(r,g,b);
        }
            }
      setColorInterval(minColor, maxColor); // end points


            for (i = 0; i < colormapLength; i++)
            {
        r = (int)(cMatrix(i, 0) * 255);
        g = (int)(cMatrix(i, 1) * 255);
        b = (int)(cMatrix(i, 2) * 255);
        addColorStop( (m_colorLevels(i) - min) / (max - min), QColor(r, g, b));
            }
      break;
  }

}

FloodPlotData::FloodPlotData()
  : QwtRasterData()
{

}

FloodPlotData::~FloodPlotData()
{

}

TimeSeriesFloodPlotData::TimeSeriesFloodPlotData(TimeSeries timeSeries)
: FloodPlotData(),
  m_timeSeries(timeSeries),
  m_minValue(minimum(timeSeries.values())),
  m_maxValue(maximum(timeSeries.values())),
  m_minX(timeSeries.firstReportDateTime().date().dayOfYear()),
  m_maxX(ceil(timeSeries.daysFromFirstReport()[timeSeries.daysFromFirstReport().size()-1]+timeSeries.firstReportDateTime().date().dayOfYear()+timeSeries.firstReportDateTime().time().totalDays())), // end day
  m_minY(0), // start hour
  m_maxY(24), // end hour
  m_startFractionalDay(timeSeries.firstReportDateTime().date().dayOfYear()+timeSeries.firstReportDateTime().time().totalDays()),
  m_colorMapRange(QwtInterval(m_minValue, m_maxValue))
{
  // data range
  //setBoundingRect(QRectF(m_minX, m_minY, m_maxX-m_minX, m_maxY-m_minY));
  setInterval(Qt::XAxis, QwtInterval(m_minX, m_maxX - m_minX));
  setInterval(Qt::YAxis, QwtInterval(m_minY, m_maxY - m_minY));
  setInterval(Qt::ZAxis, m_colorMapRange);
  m_units = timeSeries.units();
}

TimeSeriesFloodPlotData::TimeSeriesFloodPlotData(TimeSeries timeSeries,  QwtInterval colorMapRange)
: FloodPlotData(),
  m_timeSeries(timeSeries),
  m_minValue(minimum(timeSeries.values())),
  m_maxValue(maximum(timeSeries.values())),
  m_minX(timeSeries.firstReportDateTime().date().dayOfYear()),
  m_maxX(ceil(timeSeries.daysFromFirstReport()[timeSeries.daysFromFirstReport().size()-1]+timeSeries.firstReportDateTime().date().dayOfYear()+timeSeries.firstReportDateTime().time().totalDays())), // end day
  m_minY(0), // start hour
  m_maxY(24), // end hour
  m_startFractionalDay(timeSeries.firstReportDateTime().date().dayOfYear()+timeSeries.firstReportDateTime().time().totalDays()),
  m_colorMapRange(colorMapRange)
{
  // data range
  //setBoundingRect(QRectF(m_minX, m_minY, m_maxX-m_minX, m_maxY-m_minY));
  setInterval(Qt::XAxis, QwtInterval(m_minX, m_maxX - m_minX));
  setInterval(Qt::YAxis, QwtInterval(m_minY, m_maxY - m_minY));
  setInterval(Qt::ZAxis, m_colorMapRange);
  m_units = timeSeries.units();
}

TimeSeriesFloodPlotData* TimeSeriesFloodPlotData::copy() const
{
  TimeSeriesFloodPlotData* result = new TimeSeriesFloodPlotData(m_timeSeries, m_colorMapRange);
  return result;
}

TimeSeriesFloodPlotData::~TimeSeriesFloodPlotData()
{

}

QwtInterval TimeSeriesFloodPlotData::range() const 
{ 
  return m_colorMapRange; 
}

QRectF TimeSeriesFloodPlotData::boundingRect() const
{
  return QRectF(m_minX, m_minY, m_maxX-m_minX, m_maxY-m_minY);
}

double TimeSeriesFloodPlotData::value(double fractionalDay, double hourOfDay) const
{
  // DLM: we are flooring the day because we want to plot day vs hour in flood plot
  double fracDays = floor(fractionalDay) + hourOfDay/24.0;
  return m_timeSeries.value(fracDays-m_startFractionalDay);
}

/// minX
double TimeSeriesFloodPlotData::minX() const { return m_minX; };

/// maxX
double TimeSeriesFloodPlotData::maxX() const { return m_maxX; };

/// minY
double TimeSeriesFloodPlotData::minY() const { return m_minY; };

/// maxY
double TimeSeriesFloodPlotData::maxY() const { return m_maxY; };

/// minValue
double TimeSeriesFloodPlotData::minValue() const { return m_minValue; };

/// maxValue
double TimeSeriesFloodPlotData::maxValue() const { return m_maxValue; };

/// minValue setter
void TimeSeriesFloodPlotData::minValue(double min) { m_minValue = min; };

/// maxValue setter
void TimeSeriesFloodPlotData::maxValue(double max) { m_maxValue = max; };

/// sumValue
double TimeSeriesFloodPlotData::sumValue() const
{
  return sum(m_timeSeries.values());
}

/// meanValue
double TimeSeriesFloodPlotData::meanValue() const
{
  return mean(m_timeSeries.values());
}

/// stdDevValue
double TimeSeriesFloodPlotData::stdDevValue() const
{
  return stdDev(m_timeSeries.values());
}

void TimeSeriesFloodPlotData::colorMapRange(QwtInterval colorMapRange) 
{ 
  m_colorMapRange = colorMapRange; 
}

QwtInterval TimeSeriesFloodPlotData::colorMapRange() 
{ 
  return m_colorMapRange; 
}

std::string TimeSeriesFloodPlotData::units() const 
{ 
  return m_units; 
};

MatrixFloodPlotData::MatrixFloodPlotData(const Matrix& matrix)
: FloodPlotData(),
  m_xVector(linspace(0, matrix.size1() - 1, matrix.size1())),
  m_yVector(linspace(0, matrix.size2()-1, matrix.size2())),
  m_matrix(matrix),
  m_interpMethod(NearestInterp)
{
  init();
}

MatrixFloodPlotData::MatrixFloodPlotData(const Matrix& matrix,  QwtInterval colorMapRange)
: FloodPlotData(),
  m_xVector(linspace(0, matrix.size1() - 1, matrix.size1())),
  m_yVector(linspace(0, matrix.size2()-1, matrix.size2())),
  m_matrix(matrix),
  m_interpMethod(NearestInterp)
{
  init();
  m_colorMapRange = colorMapRange;
}



MatrixFloodPlotData::MatrixFloodPlotData(const Vector& xVector,
                                         const Vector& yVector,
                                         const Matrix& matrix)
: FloodPlotData(),
  m_xVector(xVector),
  m_yVector(yVector),
  m_matrix(matrix),
  m_interpMethod(NearestInterp)
{
  init();
}


MatrixFloodPlotData::MatrixFloodPlotData(const Vector& xVector,
                                         const Vector& yVector,
                                         const Matrix& matrix,
                                         const InterpMethod interp)
: FloodPlotData(),
  m_xVector(xVector),
  m_yVector(yVector),
  m_matrix(matrix),
  m_interpMethod(interp)
{
  init();
}

MatrixFloodPlotData::MatrixFloodPlotData(const std::vector<double>& xVector,
                                         const std::vector<double>& yVector,
                                         const std::vector<double>& matrix)
: FloodPlotData(),
  m_xVector(xVector.size()),
  m_yVector(yVector.size()),
  m_matrix(xVector.size(),yVector.size()),
  m_interpMethod(NearestInterp)
{
  std::copy(xVector.begin(),xVector.end(),m_xVector.begin());
  std::copy(yVector.begin(),yVector.end(),m_yVector.begin());
  for (unsigned j=0;j<yVector.size();j++)
    for (unsigned i=0;i<xVector.size();i++)
      m_matrix(i,j)=matrix[j*xVector.size()+i];
  init();
}


MatrixFloodPlotData::MatrixFloodPlotData(const std::vector<double>& xVector,
                                         const std::vector<double>& yVector,
                                         const std::vector<double>& matrix,
                                         const InterpMethod interp)
: FloodPlotData(), 
  m_xVector(xVector.size()),
  m_yVector(yVector.size()),
  m_matrix(xVector.size(),yVector.size()),
  m_interpMethod(interp)
{
  std::copy(xVector.begin(),xVector.end(),m_xVector.begin());
  std::copy(yVector.begin(),yVector.end(),m_yVector.begin());
  for (unsigned j=0;j<yVector.size();j++)
    for (unsigned i=0;i<xVector.size();i++)
      m_matrix(i,j)=matrix[j*xVector.size()+i];
  init();
}




MatrixFloodPlotData::MatrixFloodPlotData(const Vector& xVector,
                                         const Vector& yVector,
                                         const Matrix& matrix,
                                         QwtInterval colorMapRange)
: FloodPlotData(),
  m_xVector(xVector),
  m_yVector(yVector),
  m_matrix(matrix),
  m_interpMethod(NearestInterp)
{
  init();
  m_colorMapRange = colorMapRange;
}

MatrixFloodPlotData::~MatrixFloodPlotData()
{

}

MatrixFloodPlotData* MatrixFloodPlotData::copy() const
{
  auto result = new MatrixFloodPlotData(m_xVector, m_yVector, m_matrix, m_colorMapRange);
  result->interpMethod(m_interpMethod);
  return result;
}

QwtInterval MatrixFloodPlotData::range() const { return m_colorMapRange; }

double MatrixFloodPlotData::value(double x, double y) const
{
  return interp(m_xVector, m_yVector, m_matrix, x, y, m_interpMethod, NearestExtrap);
}

/// set the interp method, defaults to Nearest
void MatrixFloodPlotData::interpMethod(InterpMethod interpMethod)
{
  m_interpMethod = interpMethod;
}

/// minX
double MatrixFloodPlotData::minX() const
{
  return m_minX;
}

/// maxX
double MatrixFloodPlotData::maxX() const
{
  return m_maxX;
}

/// minY
double MatrixFloodPlotData::minY() const
{
  return m_minY;
}

/// maxY
double MatrixFloodPlotData::maxY() const
{
  return m_maxY;
}

/// minValue
double MatrixFloodPlotData::minValue() const { return m_minValue; };

/// maxValue
double MatrixFloodPlotData::maxValue() const { return m_maxValue; };

/// minValue setter
void MatrixFloodPlotData::minValue(double min) { m_minValue = min; };

/// maxValue setter
void MatrixFloodPlotData::maxValue(double max) { m_maxValue = max; };

/// sumValue
double MatrixFloodPlotData::sumValue() const
{
  return sum(m_matrix);
}

/// meanValue
double MatrixFloodPlotData::meanValue() const
{
  return mean(m_matrix);
}

/// stdDevValue
double MatrixFloodPlotData::stdDevValue() const
{
  return 0;
}

/// range of values for which to show the colormap
void MatrixFloodPlotData::colorMapRange(QwtInterval colorMapRange) { m_colorMapRange = colorMapRange; }

/// range of values for which to show the colormap
QwtInterval MatrixFloodPlotData::colorMapRange() { return m_colorMapRange; }

/// units for plotting on axes or scaling
std::string MatrixFloodPlotData::units() const { return m_units; };

// set ranges and bounding box
void MatrixFloodPlotData::init(){

  unsigned M = m_matrix.size1();
  unsigned N = m_matrix.size2();

  if ((M <= 1) || (N <= 1) || (M != m_xVector.size()) || (N != m_yVector.size())){
    throw std::runtime_error("Incorrectly sized matrix or vector for MatrixFloodPlotData");
  }

  m_minX = minimum(m_xVector);
  m_maxX = maximum(m_xVector);
  m_minY = minimum(m_yVector);
  m_maxY = maximum(m_yVector);

  m_minValue = minimum(m_matrix);
  m_maxValue = maximum(m_matrix);

  // default behavior is to have entire data range considered for colormapping
  // override by setting colorMapRange
  m_colorMapRange = QwtInterval(m_minValue, m_maxValue);

  // set the bounding box
  //setBoundingRect(QRectF(m_minX, m_minY, m_maxX-m_minX, m_maxY-m_minY));
  setInterval(Qt::XAxis, QwtInterval(m_minX, m_maxX - m_minX));
  setInterval(Qt::YAxis, QwtInterval(m_minY, m_maxY - m_minY));
  setInterval(Qt::ZAxis, m_colorMapRange);
}


/* --------------------------------------
 * FloodPlot Class
 * --------------------------------------
*/

FloodPlot::FloodPlot(QWidget* parent, Qt::WindowFlags flags) 
  : Plot2D(parent, flags)
{
  init();
}

FloodPlot::FloodPlot(const Matrix& m, QWidget* parent, Qt::WindowFlags flags)
  : Plot2D(parent, flags)
{
  init();

  // deleted by internals of floodPlotData
  MatrixFloodPlotData* data = new MatrixFloodPlotData(m);
  data->interpMethod(LinearInterp);
  floodPlotData(data);
}

FloodPlot::~FloodPlot()
{

}

void FloodPlot::init()
{
  m_plot2DTimeAxis = nullptr;
  // destroy windows when closed
  setAttribute(Qt::WA_DeleteOnClose);

  // deleted by m_qwtPlot after attach
  m_spectrogram = new QwtPlotSpectrogram();
  m_colorMapType = FloodPlotColorMap::Jet;
  m_colorMapLength = 64;
  m_spectrogram->attach(m_qwtPlot);
  m_qwtPlot->plotLayout()->setAlignCanvasToScales(true);

  // TODO - base on data
  this->leftAxisTitle("Hour of Day");
  this->bottomAxisTitle("Day of Simulation");
}

void FloodPlot::selectColorMap()
{// cycle through list
  switch (m_colorMapType)
  {
    case FloodPlotColorMap::Jet:
      {
        colorMap(FloodPlotColorMap::Gray);
        break;
      }
    case FloodPlotColorMap::Gray:
      {
        colorMap(FloodPlotColorMap::Jet);
        break;
      }
  };
  m_qwtPlot->replot();
}

void FloodPlot::showContourPlot()
{
  if (contourPlot->text() == "Show Contour")
  {
    contourPlot->setText(tr("Show Colormap"));
    showSpectrogram(false);
    showContour(true);
  }
  else
  {
    contourPlot->setText(tr("Show Contour"));
    showContour(false);
    showSpectrogram(true);
  }
  m_qwtPlot->replot();
}


void FloodPlot::timeseriesData(TimeSeries tsData)
{
  if (tsData.values().empty()){
    return;
  }

  m_startDateTime = tsData.firstReportDateTime();
  m_endDateTime = tsData.firstReportDateTime() + Time(tsData.daysFromFirstReport(tsData.daysFromFirstReport().size()-1));
  m_duration = (m_endDateTime-m_startDateTime).totalDays();
  m_xAxisMin = 0.0;
  m_xAxisMax = m_duration;
  if (m_plot2DTimeAxis == nullptr) 
  {
    // deleted after setAxisScaleDraw
    m_plot2DTimeAxis = new Plot2DTimeAxis(m_startDateTime, m_duration);
    m_qwtPlot->setAxisTitle(QwtPlot::xBottom, " Simulation Time");
    m_qwtPlot->setAxisScale(QwtPlot::xBottom, 0, m_duration);
    m_qwtPlot->setAxisScaleDraw(QwtPlot::xBottom, m_plot2DTimeAxis);
    m_qwtPlot->setAxisLabelRotation(QwtPlot::xBottom, -90.0);
    m_qwtPlot->setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
  } 
  else 
  {
    m_plot2DTimeAxis->startDateTime(m_startDateTime);
    m_plot2DTimeAxis->duration(m_duration);
  }

  // deleted by the internals of floodPlotData
  TimeSeriesFloodPlotData* data = new TimeSeriesFloodPlotData(tsData);
  floodPlotData(data);
}

void FloodPlot::floodPlotData(FloodPlotData* data)
{
  if (!data){
    return;
  }

  m_qwtPlot->setAxisScale(QwtPlot::xBottom, data->minX(), data->maxX());
  m_qwtPlot->setAxisScale(QwtPlot::yLeft, data->minY(), data->maxY());

  rightAxisTitleFromUnits(data->units());
  m_spectrogram->setData(data); // takes ownership of data

  dataAutoRange();

  colorMap(m_colorMapType);

/// default contour levels - 10
  QList<double> contourLevels;
  double interval = (data->maxValue() - data->minValue()) / 10.0;
  for (double level = data->minValue(); level < data->maxValue(); level += interval){
    contourLevels += level;
  }

  m_spectrogram->setContourLevels(contourLevels);
  m_qwtPlot->replot();
}

void FloodPlot::contourLevels(Vector& contourValues)
{
  QList<double> contours;
  for (unsigned int level = 0; level < contourValues.size(); level++){
    contours += contourValues(level);
  }

  m_spectrogram->setContourLevels(contours);
  m_qwtPlot->replot();
}

void FloodPlot::colorLevels(Vector& colorLevels)
{
  // update color stops based on input vector

  m_colorLevels = colorLevels;

  FloodPlotColorMap* colorMap = new FloodPlotColorMap(m_colorLevels, m_colorMapType);
  m_spectrogram->setColorMap(colorMap);
  m_qwtPlot->setAxisScale(QwtPlot::yRight, minimum(colorLevels), maximum(colorLevels)); // legend numbers

  colorMap = new FloodPlotColorMap(m_colorLevels, m_colorMapType);
  m_rightAxis->setColorMap(QwtInterval(minimum(colorLevels), maximum(colorLevels)), colorMap); // legend colors
  m_qwtPlot->replot();
}

void FloodPlot::colorMapRange(double min, double max)
{
  if (min >= max){
    return;
  }

  Vector levels(2);
  levels(0) = min;
  levels(1) = max;
  colorLevels(levels);
}

void FloodPlot::showContour(bool on)
{
  m_spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, on);
}

void FloodPlot::showSpectrogram(bool on)
{
  m_spectrogram->setDisplayMode(QwtPlotSpectrogram::ImageMode, on);
  m_spectrogram->setDefaultContourPen(on ? QPen() : QPen(Qt::NoPen));
}

void FloodPlot::colorMap(FloodPlotColorMap::ColorMapList clrMap)
{
  m_colorMapType = clrMap;
  initColorMap();
  initColorBar();
}

void FloodPlot::initColorMap()
{
  FloodPlotColorMap* colorMap = new FloodPlotColorMap(m_colorLevels, m_colorMapType);
  m_spectrogram->setColorMap(colorMap);
}

void FloodPlot::dataAutoRange()
{
  m_dataRange = m_spectrogram->data()->interval(Qt::XAxis);
  if (m_dataRange.minValue() == m_dataRange.maxValue()){
    m_dataRange = QwtInterval(m_dataRange.minValue(), m_dataRange.minValue() + 1.0);
  }

  m_colorLevels=linspace(m_dataRange.minValue(), m_dataRange.maxValue(),m_colorMapLength);
}

void FloodPlot::initColorBar()
{
  m_rightAxis = m_qwtPlot->axisWidget(QwtPlot::yRight);
  m_rightAxis->setColorBarEnabled(true);

  FloodPlotColorMap* colorMap = new FloodPlotColorMap(m_colorLevels, m_colorMapType);
  m_rightAxis->setColorMap(m_dataRange, colorMap);

  m_qwtPlot->setAxisScale(QwtPlot::yRight, m_dataRange.minValue(), m_dataRange.maxValue() );
  m_qwtPlot->enableAxis(QwtPlot::yRight);

}

void FloodPlot::xCenterSpan(double center, double span)
{
  /// min = maximum(center - span, min(x data))
  /// max = minimum(center + span, max(x data))
  double centerX = centerXValue(center);
  double spanX = spanXValue(span);
  double minX = centerX - spanX;
  double maxX = centerX + spanX;
  if (minX < m_xAxisMin)  {
    minX = m_xAxisMin;
    maxX = minX + 2.0 * spanX;
  } else if (maxX > m_xAxisMax) {
    maxX = m_xAxisMax;
    minX = maxX - 2.0 * spanX;
  }
  m_qwtPlot->setAxisScale(QwtPlot::xBottom, minX, maxX);
  m_qwtPlot->replot();
}

void FloodPlot::dragEnterEvent(QDragEnterEvent *e) 
{ 
  e->accept(); 
}

double FloodPlot::centerXValue(double center)
{
  /// do any translations here
  return center;
}

double FloodPlot::spanXValue(double span)
{
  /// do any translations here
  return span;
}


} // openstudio
