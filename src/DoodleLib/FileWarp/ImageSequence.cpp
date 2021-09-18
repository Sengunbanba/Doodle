#include <DoodleLib/Exception/Exception.h>
#include <DoodleLib/FileWarp/ImageSequence.h>
#include <DoodleLib/core/CoreSet.h>
#include <DoodleLib/core/DoodleLib.h>
#include <DoodleLib/threadPool/ThreadPool.h>
#include <Logger/Logger.h>
#include <Metadata/Episodes.h>
#include <Metadata/Shot.h>
#include <PinYin/convert.h>
#include <core/DoodleLib.h>

#include <boost/algorithm/string.hpp>
#include <opencv2/opencv.hpp>
namespace doodle {
std::string ImageSequence::clearString(const std::string &str) {
  auto &con  = convert::Get();
  auto str_r = std::string{};
  str_r      = con.toEn(str);

  return str_r;
}
ImageSequence::ImageSequence()
    : p_paths(),
      p_Text(),
      p_long_sig(std::make_shared<long_term>()) {
}
ImageSequence::ImageSequence(const FSys::path &path_dir, const std::string &text)
    : std::enable_shared_from_this<ImageSequence>(),
      p_paths(),
      p_Text(std::move(clearString(text))),
      stride(),
      p_long_sig(std::make_shared<long_term>()) {
  set_path(path_dir);
}

bool ImageSequence::hasSequence() {
  return !p_paths.empty();
}

void ImageSequence::set_path(const FSys::path &dir) {
  this->seanDir(dir);
  for (auto &path : p_paths) {
    if (!FSys::is_regular_file(path)) {
      throw DoodleError("不是文件, 无法识别");
    }
  }
}

long_term_ptr ImageSequence::get_long_term() {
  return p_long_sig;
}

bool ImageSequence::seanDir(const FSys::path &dir) {
  if (!FSys::is_directory(dir))
    throw FileError{dir, "file not is a directory"};

  FSys::path ex{};
  for (auto &path : FSys::directory_iterator(dir)) {
    if (FSys::is_regular_file(path)) {
      if (ex.empty()) {
        ex = path.path().extension();
      }
      if (path.path().extension() == ex) {
        p_paths.emplace_back(path.path());
      }
    }
  }
  if (p_paths.empty())
    throw DoodleError("空目录");
  return true;
}

void ImageSequence::setText(const std::string &text) {
  p_Text = clearString(text);
}

void ImageSequence::createVideoFile(const FSys::path &out_file) {
  if (!out_file.empty())
    p_out_path = out_file;

  if (!this->hasSequence())
    throw DoodleError{"not Sequence"};
  std::this_thread::sleep_for(std::chrono::milliseconds{10});
  //检查父路径存在
  if (!FSys::exists(p_out_path.parent_path()))
    FSys::create_directories(p_out_path.parent_path());

  {
    const static cv::Size k_size{1280, 720};
    auto video           = cv::VideoWriter{p_out_path.generic_string(),
                                 cv::VideoWriter::fourcc('D', 'I', 'V', 'X'),
                                 25,
                                 cv::Size(1280, 720)};
    auto k_image         = cv::Mat{};
    auto k_image_resized = cv::Mat{};
    auto k_clone         = cv::Mat{};

    auto k_size_len = p_paths.size();

    //排序图片
    std::sort(p_paths.begin(), p_paths.end(),
              [](const FSys::path &k_r, const FSys::path &k_l) -> bool { return k_r.stem() < k_l.stem(); });

    for (auto &&path : p_paths) {
      k_image = cv::imread(path.generic_string());
      if (k_image.empty())
        throw DoodleError("open cv not read image");
      if (k_image.cols != 1280 || k_image.rows != 720)
        cv::resize(k_image, k_image_resized, k_size);
      else
        k_image_resized = k_image;

      {  //创建水印
        k_clone          = k_image_resized.clone();
        int fontFace     = cv::HersheyFonts::FONT_HERSHEY_COMPLEX;
        double fontScale = 1;
        int thickness    = 2;
        int baseline     = 0;
        auto textSize    = cv::getTextSize(p_Text, fontFace,
                                           fontScale, thickness, &baseline);
        baseline += thickness;
        textSize.width += baseline;
        textSize.height += baseline;
        // center the text
        cv::Point textOrg((k_image_resized.cols - textSize.width) / 8,
                          (k_image_resized.rows + textSize.height) / 8);

        // draw the box
        cv::rectangle(k_clone, textOrg + cv::Point(0, baseline),
                      textOrg + cv::Point(textSize.width, -textSize.height),
                      cv::Scalar(0, 0, 0), -1);

        cv::addWeighted(k_clone, 0.7, k_image_resized, 0.3, 0, k_image_resized);
        // then put the text itself
        cv::putText(k_image_resized, p_Text, textOrg, fontFace, fontScale,
                    cv::Scalar{0, 255, 255}, thickness, cv::LineTypes::LINE_AA);
      }

      p_long_sig->sig_progress(rational_int{1, k_size_len});

      video << k_image_resized;
    }
  }
  p_long_sig->sig_finished();
  p_long_sig->sig_message_result(fmt::format("成功创建视频 {}", p_out_path));
}

long_term_ptr ImageSequence::create_video_asyn(const FSys::path &out_file) {
  auto k_fut = DoodleLib::Get().get_thread_pool()->enqueue(
      [this, out_file]() { this->createVideoFile(out_file); });
  p_long_sig->p_list.push_back(std::move(k_fut));
  return p_long_sig;
}
std::string ImageSequence::set_shot_and_eps(const ShotPtr &in_shot, const EpisodesPtr &in_episodes) {
  auto k_str = CoreSet::getSet().getUser_en();  /// 基本水印, 名称
  /// 如果可以找到集数和镜头号直接添加上去, 否者就这样了
  if (in_shot && in_episodes) {
    k_str += fmt::format(" : {}_{}", in_episodes->str(), in_shot->str());
  } else if (in_shot) {
    k_str += fmt::format(" : {}", in_shot->str());
  } else if (in_episodes) {
    k_str += fmt::format(" : {}", in_episodes->str());
  }
  p_Text = k_str;

  /// 添加文件路径名称
  boost::replace_all(k_str, " ", "_");  /// 替换不好的文件名称组件
  boost::replace_all(k_str, ":", "_");  /// 替换不好的文件名称组件
  k_str += ".mp4";
  p_out_path = p_paths.front().parent_path().parent_path();
  if (in_episodes)
    p_out_path /= in_episodes->str();
  p_out_path /= k_str;

  return p_Text;
}

// ImageSequenceBatch::ImageSequenceBatch(decltype(p_paths) dirs)
//     : LongTerm(),
//       p_paths(std::move(dirs)),
//       p_imageSequences() {
//   for (auto &&dir : p_paths) {
//     if (FSys::is_directory(dir))
//       p_imageSequences.emplace_back(
//           std::make_shared<ImageSequence>(dir));
//     else
//       throw DoodleError("不是目录");
//   }
// }
//
// ImageSequenceBatch::ImageSequenceBatch(decltype(p_imageSequences) imageSequences)
//     : p_paths(),
//       p_imageSequences(std::move(imageSequences)) {
//   for (auto &&image : p_imageSequences) {
//     p_paths.emplace_back(image->getDir());
//   }
// }
//
// void ImageSequenceBatch::batchCreateSequence(const FSys::path &out_dir) const {
//   if (p_imageSequences.empty()) return;
//   auto &set = CoreSet::getSet();
//   //这里使用序列图的父路径加uuid防止重复
//   auto k_path = p_imageSequences[0]->getDir().parent_path() /
//                 boost::uuids::to_string(set.getUUID());
//   // auto k_path = set.getCacheRoot() / boost::uuids::to_string(set.getUUID());
//
//   //创建生成路径
//   if (!out_dir.empty())
//     k_path = out_dir;
//   //检查路径存在
//   if (!FSys::exists(k_path))
//     FSys::create_directories(k_path);
//
//   //创建线程池, 开始
//   auto k_pool = DoodleLib::Get().get_thread_pool();
//   std::map<FSys::path, std::future<void>> result{};
//   //创建锁
//   std::recursive_mutex p_mutex{};
//   auto k_i = float{1};
//   //添加进度回调函数
//   auto k_add_fun = std::bind<void>([&p_mutex](float i, float *_1) {
//     std::unique_lock lock{p_mutex};
//     (*_1) += i;
//   },
//                                    std::placeholders::_1, &k_i);
//   for (const auto &im : p_imageSequences) {
//     auto str = im->getEpisodesAndShot_str().append(".mp4");
//     im->stride.connect(k_add_fun);
//
//     result.emplace(im->getDir(),
//                    k_pool->enqueue(
//                        [k_path, str, im] {
//                          // !从这里开始送入线程池, 防止线程检查重名式失败
//                          //检查存在,如果存在就使用其他名称
//                          auto path = k_path / str;
//                          if (FSys::exists(path) || str == "ep0000_sc0000") {
//                            path.remove_filename();
//                            path /= boost::uuids::to_string(CoreSet::getSet().getUUID()).append(".mp4");
//                          }
//                          im->createVideoFile(path);
//                        }));
//   }
//   std::future_status status{};
//   auto it          = result.begin();
//   const auto k_len = boost::numeric_cast<float>(p_imageSequences.size());
//
//   while (!result.empty()) {
//     status = it->second.wait_for(std::chrono::milliseconds{10});
//     {
//       std::unique_lock lock{p_mutex};
//       this->progress(boost::numeric_cast<int>(k_i / k_len));
//     }
//     if (status == std::future_status::ready) {
//       ++k_i;
//
//       std::string mess{"成功"};
//       try {
//         it->second.get();
//       } catch (const DoodleError &err) {
//         mess = fmt::format("失败: {}", err.what());
//         DOODLE_LOG_INFO(mess)
//       }
//
//       this->messagResult(mess);
//       //这里要擦除数据
//       it = result.erase(it);
//     } else {
//       //超时后继续等待其他
//       ++it;
//     }
//     // 如果到了结尾就返回开始
//     if (it == result.end()) {
//       it = result.begin();
//     }
//   }
//   this->finished();
// }

}  // namespace doodle
