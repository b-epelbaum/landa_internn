#pragma once
#include <memory>
#include "jutils.h"
#include "applog.h"

#include "RealTimeStats.h"
#include "frameRef.h"
#include "BackgroundThreadPool.h"
#include <iomanip>
#include "TaskThreadPool.h"

namespace LandaJune 
{
	namespace Functions
	{
		static void frameCreateRegions(const FrameProviders::FrameRef *frame)
		{
			const auto& imageFormat = frame->getFormat();

			if (imageFormat == std::nullopt)
			{
				PRINT_ERROR << "fcopy : format is not supported : " << QString::fromStdString(frame->getFormatString());
				RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_createdRegionsFail, 1.0e-6);
				return;
			}

			const auto t0 = Utility::now_in_microseconds();
			QImage image = QImage(static_cast<const uchar*>(frame->getBits()), frame->getWidth(), frame->getHeight(), imageFormat.value());
			const auto t1 = Utility::now_in_microseconds();
			auto diff = (t1 - t0) * 1.0e-3;
			PRINT_INFO3 << "===========================================";
			PRINT_INFO3 << "QImage creation : " << diff << " msec";
			PRINT_INFO3 << "===========================================";

			const QRect region1{ 0, 0, 400, static_cast<int>(frame->getHeight()) };
			const QRect region2{ static_cast<int>(frame->getWidth()) - 400, 0, 400, static_cast<int>(frame->getHeight()) };
			auto& pool = Threading::TaskThreadPools::algorithmsThreadPool();

			auto copyRegIon = [](const QImage& srcImage, QRect reg, QImage& retImage)
			{
				retImage = srcImage.copy(reg);
			};


			QImage regImageLeft, regImageRight;
			std::vector<Threading::TaskThreadPool::JobFuture<void>> _futureList;


			_futureList.push_back(Threading::TaskThreadPools::postJob(pool, copyRegIon, image, region1, std::ref(regImageLeft)));
			_futureList.push_back(Threading::TaskThreadPools::postJob(pool, copyRegIon, image, region2, std::ref(regImageRight)));

			std::for_each(_futureList.begin(), _futureList.end(), [](auto &f) { f.wait(); });

			//_futureList[0].get();


			//copyRegIon(image, region1, std::ref(regImageLeft));
			//copyRegIon(image, region2, std::ref(regImageRight));


			/*

			for (int i = 0; i < 100; i++)
			{
			//auto regImage = image.copy(region);

			auto regImage = QImage(region.width(), region.height(), format.value());
			const auto fres = !!regImage.bits();
			if (fres)
			{
			const size_t bpl = regImage.bytesPerLine();
			const auto fbpl = frame->getWidth() * bpl / region.width();

			auto dst = regImage.bits();
			const uchar *src = frame->getBits();
			src += region.top() * fbpl;
			src += region.left() * fbpl / frame->getWidth();
			for (auto t = region.top(); t < region.bottom(); ++t)
			{
			memcpy(dst, src, bpl);
			src += fbpl;
			dst += bpl;
			}
			}
			}
			*/


			const auto t2 = Helpers::Utility::now_in_microseconds();
			diff = (t2 - t1) * 1.0e-3;
			PRINT_INFO3 << "===========================================";
			PRINT_INFO3 << "Region copy creation : " << diff << " msec";
			PRINT_INFO3 << "===========================================";

			bool res = false;
			res = regImageLeft.save("d:\\temp\\regionLeft.bmp");
			res = regImageRight.save("d:\\temp\\regionRight.bmp");

			const auto t3 = Helpers::Utility::now_in_microseconds();
			diff = (t3 - t2) * 1.0e-3;
			PRINT_INFO3 << "===========================================";
			PRINT_INFO3 << "Region Image save : " << diff << " msec";
			PRINT_INFO3 << "===========================================";

			/*
			auto const& pSrcData = frame->getBits();
			uint8_t * TargetData = reinterpret_cast<uint8_t*>(image.bits());
			auto const& imgSize = image.sizeInBytes();
			std::memcpy(TargetData, pSrcData, imgSize);

			const auto t1 = Utility::now_in_microseconds();
			auto diff = (t1 - t0) * 1.0e-6;
			PRINT_INFO3 << "===========================================";
			PRINT_INFO3 << "Memory copy : " << diff << " msec";
			PRINT_INFO3 << "===========================================";
			*/
			res = image.save("d:\\temp\\LARGE.bmp");
			const auto t4 = Helpers::Utility::now_in_microseconds();
			diff = (t4 - t3) * 1.0e-3;
			PRINT_INFO3 << "===========================================";
			PRINT_INFO3 << "Whole Image save : " << diff << " msec";
			PRINT_INFO3 << "===========================================";



			/*
			auto const& imgSize = image.sizeInBytes();

			std::vector<uint8_t> srcVec(pSrcData, pSrcData + image.sizeInBytes());
			std::vector<uint8_t> tarVec(TargetData, TargetData + image.sizeInBytes());

			//std::execution::par_unseq,
			std::copy(srcVec.begin(), srcVec.begin() + 100, std::back_inserter(tarVec));

			//			std::copy(image.bits(), const_cast<uchar*>(frame->getBits()), image.sizeInBytes());
			const auto t1 = Utility::now_in_microseconds();
			auto diff = (t1 - t0) * 1.0e-6;

			/*
			std::ostringstream ss;
			ss << std::setw(6) << std::setfill('0') << frame->getIndex();


			for ( auto const& region : frame->getParams()->inputParams().regions())
			{
			const auto& regWidth = region.width();
			const auto& regHeight = region.height();

			auto prefix = pParams->inputParams().targetFolder().toStdString();
			prefix.append("\\frame_")
			.append(ss.str())
			.append("_region_[")
			.append(std::to_string(region.left()))
			.append(", ")
			.append(std::to_string(region.top()))
			.append(", (")
			.append(std::to_string(regWidth))
			.append("x")
			.append(std::to_string(regHeight))
			.append(")]_(")
			.append(std::to_string(Utility::now_in_millisecond()))
			.append(")");

			const auto t0 = Utility::now_in_microseconds();

			//QImage image(regWidth, regHeight, format.value());

			image = QImage (regWidth, regHeight, format.value());
			const auto fres = !!image.bits();
			if (fres)
			{
			const size_t bpl = image.bytesPerLine();
			const auto fbpl = frame->getWidth() * bpl / regWidth;

			auto dst = image.bits();
			const uchar *src = frame->getBits();
			src += region.top() * fbpl;
			src += region.left() * fbpl / frame->getWidth();
			for (auto t = region.top(); t < region.bottom(); ++t)
			{
			memcpy(dst, src, bpl);
			src += fbpl;
			dst += bpl;
			}
			}

			const auto t1 = Utility::now_in_microseconds();

			auto diff = (t1 - t0) * 1.0e-6;
			image.save("d:\\temp\\LARGE.bmp");

			if (fres)
			{
			RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_createdRegionsOk, (t1 - t0) * 1.0e-6);

			auto pathName = prefix;
			pathName.append(".bmp");

			strPath = pathName;
			//saveBitmapPool().call(frameSaveImage, std::move(image), pathName);
			}
			else
			{
			RealTimeStats::rtStats()->increment(RealTimeStats::objectsPerSec_createdRegionsFail, (t1 - t0) * 1.0e-6);
			}
			}
			*/
		}
	}
}
