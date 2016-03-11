#include <sstream>

#include <QByteArray>
#include <QDebug>
#include <QJsonDocument>

#include <boost/algorithm/string.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/stream.hpp>

#include "kancolledatabase.h"

namespace {
  std::string DecompressResponseBody(const std::string& response) {
    size_t start = 0;
    std::string content_encoding = "";
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
    }

    std::string compressed_data = response.substr(start);
    if (content_encoding == "" || content_encoding == "identity") {
      return response.substr(start);
    } else if (content_encoding == "gzip") {
      boost::iostreams::array_source source(response.c_str() + start,
                                            response.length() - start);
      std::ostringstream oss(std::ios_base::binary);
      boost::iostreams::filtering_istreambuf in;
      in.push(boost::iostreams::gzip_decompressor());
      in.push(source);
      boost::iostreams::copy(in, oss);
      return oss.str();
    } else {
      qFatal("Content-Encoding %s not implemented", content_encoding.c_str());
      return "";
    }
  }
}

KanColleDatabase& KanColleDatabase::GetInstance() {
  static KanColleDatabase obj;
  return obj;
}

std::shared_ptr<const QJsonObject> KanColleDatabase::GetData() const {
  return std::atomic_load(&data_);
}

std::shared_ptr<const QJsonObject> KanColleDatabase::GetSlotItem() const {
  return std::atomic_load(&slot_item_);
}

void KanColleDatabase::ProcessData(KcsApi api_name,
                                   const std::string& response) {
  auto body = DecompressResponseBody(response);
  // + 7 byte because body begins with "svdata=..."
  auto doc = QJsonDocument::fromJson(QByteArray(body.c_str() + 7));
  Q_ASSERT(doc.isObject());

  if (api_name == KcsApi::START2) {
    auto obj = std::make_shared<const QJsonObject>(doc.object());
    std::atomic_store(&data_, obj);
  } else if (api_name == KcsApi::SLOT_ITEM) {
    auto obj = std::make_shared<const QJsonObject>(doc.object());
    std::atomic_store(&slot_item_, obj);
    emit SlotItemUpdated(this);
  }

}
