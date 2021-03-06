﻿#include <fstream>
#include <sstream>

#include <QByteArray>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>

#include <boost/algorithm/string.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/stream.hpp>

#include "httpchunkedfilter.h"
#include "kancolledatabase.h"
#include "util.h"

namespace {
  std::vector<char> DecompressResponseBody(const std::string& response) {
    size_t start = 0;
    std::string content_encoding = "", transfer_encoding = "";
    while (true) {
      auto pos = response.find("\r\n", start);
      auto line = response.substr(start, pos - start);
      start = pos + 2;
      if (line == "") break;

      std::vector<std::string> tokens;
      boost::algorithm::split(tokens, line,
                              boost::algorithm::is_space(),
                              boost::algorithm::token_compress_on);
      if (tokens[0] == "Content-Encoding:") {
        content_encoding = tokens[1];
      }
      if (tokens[0] == "Transfer-Encoding:") {
        transfer_encoding = tokens[1];
      }
    }

    boost::iostreams::array_source source(response.data() + start,
                                          response.length() - start);
    boost::iostreams::filtering_istreambuf in;

    if (content_encoding == "" || content_encoding == "identity") {
      // pass
    } else if (content_encoding == "gzip") {
      qDebug() << "data length: " << response.length() - start;
      in.push(boost::iostreams::gzip_decompressor());
    } else {
      qFatal("Content-Encoding %s not implemented", content_encoding.c_str());
    }

    if (transfer_encoding == "chunked") {
      qDebug() << "chunked";
      in.push(HttpChunkedFilter());
    } else if (transfer_encoding != "") {
      qFatal("Transfer-Encoding %s not implemented", transfer_encoding.c_str());
    }

    in.push(source);
    std::vector<char> out;

    try {
      boost::iostreams::copy(in, boost::iostreams::back_inserter(out));
    } catch (std::exception& e) {
      boost::iostreams::array_source source(response.data(),
                                            response.data() +
                                                response.length());
      std::ofstream fout("start2.gz", std::ios::binary);
      boost::iostreams::copy(source, fout);
      qFatal("%s", e.what());
    }

    out.push_back('\0');
    return out;
  }
}

KanColleDatabase& KanColleDatabase::GetInstance() {
  static KanColleDatabase obj;
  return obj;
}

std::shared_ptr<const MstData> KanColleDatabase::GetData() const {
  return std::atomic_load(&data_);
}

std::shared_ptr<const SlotItemList> KanColleDatabase::GetSlotItem() const {
  return std::atomic_load(&slot_item_);
}

std::shared_ptr<const ShipList> KanColleDatabase::GetShip() const {
  return std::atomic_load(&ship_);
}

void KanColleDatabase::ProcessData(KcsApi api_name,
                                   const std::string& response) {
  auto body = DecompressResponseBody(response);
  qDebug() << "Decompress done";
  // + 7 byte because body begins with "svdata=..."
  auto doc = QJsonDocument::fromJson(QByteArray(body.data() + 7));
  Q_ASSERT(doc.isObject());

  auto api_result = doc.object()["api_result"];
  if (api_result.toDouble(0) != 1) {
    qDebug() << "api_result not equal to 1";
    qDebug() << "value: " << api_result.toDouble();
    return;
  }

  auto api_data = doc.object()["api_data"];
  if (api_name == KcsApi::START2) {
    try {
      auto mst_data = MstData::FromJsonValue(api_data);
      std::atomic_store(&data_, mst_data);
    } catch (BadJsonValue& e) {
      qFatal(e.what());
    }
    emit DataUpdated(this);
  } else if (api_name == KcsApi::SLOT_ITEM) {
    auto new_item = std::make_shared<SlotItemList>();
    try {
      for (auto& x : api_data.toArray()) {
        new_item->push_back(SlotItem::FromJsonValue(x));
      }
    } catch (BadJsonValue& e) {
      qFatal(e.what());
    }
    std::atomic_store(&slot_item_,
                      std::const_pointer_cast<const SlotItemList>(new_item));
    emit SlotItemUpdated(this);
  } else if (api_name == KcsApi::PORT) {
    auto api_ship = api_data.toObject()["api_ship"].toArray();
    auto new_ship = std::make_shared<ShipList>();
    try {
      for (auto& x : api_ship) {
        new_ship->push_back(Ship::FromJsonValue(x));
      }
    } catch (BadJsonValue& e) {
      qFatal(e.what());
    }
    std::atomic_store(&ship_,
                      std::const_pointer_cast<const ShipList>(new_ship));
    emit ShipUpdated(this);
  }

}
