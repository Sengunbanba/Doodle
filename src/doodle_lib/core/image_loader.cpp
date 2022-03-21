//
// Created by TD on 2022/1/21.
//

#include "image_loader.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
//#include <opencv2/core/directx.hpp>

#include <metadata/project.h>
#include <metadata/image_icon.h>
#include <core/core_set.h>
#include <app/app.h>
#include <platform/win/wnd_proc.h>

//#include <DirectXTK/ScreenGrab.h>
//#include <wincodec.h>

//#include <winrt/base.h>
//#include <atlbase.h>
//#include <atlwin.h>
/// \brief 显卡驱动导入
#include <d3d11.h>
namespace doodle {
namespace {
template <class T>
struct win_ptr_delete {
  void operator()(T* ptr) const {
    if (ptr)
      ptr->Release();
  }
};
template <class T>
struct guard_win_ptr_delete {
  T* p_t;
  explicit guard_win_ptr_delete(T* in) : p_t(in) {}
  ~guard_win_ptr_delete() {
    if (p_t) p_t->Release();
  }
};

}  // namespace

class image_loader::impl {
 public:
  cache cache_p;
};

image_loader::image_loader()
    : p_i(std::make_unique<impl>()) {
  if (auto k_ctx = g_reg()->try_ctx<cache>(); !k_ctx) {
    {
      int fontFace     = cv::HersheyFonts::FONT_HERSHEY_COMPLEX;
      double fontScale = 1;
      int thickness    = 2;
      int baseline     = 0;
      {
        /// @brief 加载默认图片
        auto textSize = cv::getTextSize({"no"}, fontFace, fontScale, thickness, &baseline);
        cv::Mat k_mat{64, 64, CV_8UC4, cv::Scalar{0, 0, 0, 255}};

        cv::Point textOrg((k_mat.cols - textSize.width) * 0.5,
                          (k_mat.rows + textSize.height) * 0.5);

        cv::putText(k_mat, "no", textOrg, fontFace, fontScale,
                    {255, 255, 255, 255}, thickness, cv::LineTypes::LINE_AA);
        auto k_def                 = cv_to_d3d(k_mat);
        p_i->cache_p.default_image = k_def;
      }
    }

    {
      int fontFace     = cv::HersheyFonts::FONT_HERSHEY_COMPLEX;
      double fontScale = 1;
      int thickness    = 2;
      int baseline     = 0;
      {
        /// @brief 加载错误图片
        auto textSize = cv::getTextSize({"err"}, fontFace, fontScale, thickness, &baseline);
        cv::Mat k_mat{64, 64, CV_8UC4, cv::Scalar{0, 0, 0, 255}};

        cv::Point textOrg((k_mat.cols - textSize.width) * 0.5,
                          (k_mat.rows + textSize.height) * 0.5);

        cv::putText(k_mat, "err", textOrg, fontFace, fontScale,
                    {20, 0, 255, 255}, thickness, cv::LineTypes::LINE_AA);
        auto k_def               = cv_to_d3d(k_mat);
        p_i->cache_p.error_image = k_def;
      }
    }
    g_reg()->set<cache>(p_i->cache_p);
  } else {
    p_i->cache_p = *k_ctx;
  }
}
std::tuple<cv::Mat, std::shared_ptr<void>> image_loader::load_mat(const FSys::path& in_path) {
  const auto& l_local_path = in_path;
  if (exists(l_local_path) &&
      is_regular_file(l_local_path) &&
      (l_local_path.extension() == ".png" ||
       l_local_path.extension() == ".jpg")) {
    auto k_image = cv::imread(l_local_path.generic_string());
    chick_true<doodle_error>(!k_image.empty(), DOODLE_LOC, "open cv not read image");
    cv::cvtColor(k_image, k_image, cv::COLOR_BGR2RGBA);
    auto k_sh = cv_to_d3d(k_image, false);
    cv::cvtColor(k_image, k_image, cv::COLOR_RGBA2BGR);
    return std::make_tuple(k_image, k_sh);
  }
  return {};
}
bool image_loader::load(image_icon& in_icon, const FSys::path& in_root) {
  auto l_local_path = in_root / in_icon.path;

  auto [l_cv, l_sh] = load_mat(l_local_path);

  if (!l_cv.empty()) {
    in_icon.image   = l_sh;
    in_icon.size2d_ = l_cv.size();
  } else {
    in_icon.image = error_image();
  }

  return false;
}
bool image_loader::load(const entt::handle& in_handle) {
  chick_true<doodle_error>(in_handle.any_of<image_icon>(), DOODLE_LOC, "缺失图标组件");
  chick_true<doodle_error>(in_handle.registry()->try_ctx<project>(), DOODLE_LOC, "缺失项目上下文");
  auto& l_image     = in_handle.get<image_icon>();
  auto l_local_path = l_image.image_root(in_handle);

  load(in_handle.get<image_icon>(), l_local_path);
  in_handle.patch<image_icon>();

  return false;
}
bool image_loader::save(const entt::handle& in_handle,
                        const cv::Mat& in_image,
                        const cv::Rect2f& in_rect) {
  auto k_reg = g_reg();
  chick_true<doodle_error>(k_reg->try_ctx<project>(), DOODLE_LOC, "缺失项目上下文");

  auto& k_icon = in_handle.get_or_emplace<image_icon>();

  auto k_image = in_image(in_rect).clone();

  k_icon.path  = core_set::getSet().get_uuid_str() + ".png";
  auto k_path  = k_reg->ctx<project>().make_path("image") / k_icon.path;

  cv::imwrite(k_path.generic_string(), k_image);
  k_icon.image   = cv_to_d3d(k_image);
  k_icon.size2d_ = in_rect.size();
  return true;
}

std::shared_ptr<void> image_loader::cv_to_d3d(const cv::Mat& in_mat) const {
  return cv_to_d3d(in_mat, true);
}

cv::Mat image_loader::screenshot() {
  return win::get_screenshot();
}

std::shared_ptr<void> image_loader::default_image() const {
  return p_i->cache_p.default_image;
}
std::shared_ptr<void> image_loader::error_image() const {
  return p_i->cache_p.error_image;
}
std::shared_ptr<void> image_loader::cv_to_d3d(const cv::Mat& in_mat, bool convert_toRGBA) const {
  // 获得全局GPU渲染对象
  auto k_g = app::Get().d3dDevice;
  if (convert_toRGBA)
    /// \brief 转换图像
    cv::cvtColor(in_mat, in_mat, cv::COLOR_BGR2RGBA);

  D3D11_TEXTURE2D_DESC k_tex_desc{};
  k_tex_desc.Width            = in_mat.cols;
  k_tex_desc.Height           = in_mat.rows;
  k_tex_desc.MipLevels        = 1;
  k_tex_desc.ArraySize        = 1;
  k_tex_desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
  k_tex_desc.SampleDesc.Count = 1;
  k_tex_desc.Usage            = D3D11_USAGE_DEFAULT;
  k_tex_desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
  k_tex_desc.CPUAccessFlags   = 0;

  ID3D11Texture2D* k_com_tex{};

  D3D11_SUBRESOURCE_DATA k_sub_resource;
  k_sub_resource.pSysMem          = in_mat.data;
  k_sub_resource.SysMemPitch      = k_tex_desc.Width * 4;
  k_sub_resource.SysMemSlicePitch = 0;
  auto k_r                        = k_g->CreateTexture2D(&k_tex_desc, &k_sub_resource, &k_com_tex);
  chick_true<doodle_error>(k_r == 0, DOODLE_LOC, "windows com 异常 {}", k_r);
  guard_win_ptr_delete l_a_delete{k_com_tex};

  D3D11_SHADER_RESOURCE_VIEW_DESC k_srv;
  ZeroMemory(&k_srv, sizeof(k_srv));
  k_srv.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
  k_srv.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
  k_srv.Texture2D.MipLevels       = k_tex_desc.MipLevels;
  k_srv.Texture2D.MostDetailedMip = 0;

  ID3D11ShaderResourceView* k_out_{nullptr};
  k_r = k_g->CreateShaderResourceView(k_com_tex, &k_srv, &(k_out_));
  if (convert_toRGBA)
    /// \brief 转换图像
    cv::cvtColor(in_mat, in_mat, cv::COLOR_RGB2BGRA);
  return std::shared_ptr<void>{k_out_, win_ptr_delete<ID3D11ShaderResourceView>{}};
}
bool image_loader::save(const entt::handle& in_handle, const FSys::path& in_path) {
  auto k_reg = g_reg();
  chick_true<doodle_error>(k_reg->try_ctx<project>(), DOODLE_LOC, "缺失项目上下文");
  chick_true<doodle_error>(exists(in_path), DOODLE_LOC, "文件不存在");

  auto& k_icon = in_handle.get_or_emplace<image_icon>();
  k_icon.path  = core_set::getSet().get_uuid_str() + in_path.extension().generic_string();
  auto k_path  = k_reg->ctx<project>().make_path("image") / k_icon.path;

  FSys::copy(in_path, k_path, FSys::copy_options::overwrite_existing);
  auto l_mat = cv::imread(k_path.generic_string());
  cv::cvtColor(l_mat, l_mat, cv::COLOR_BGR2RGBA);

  k_icon.image   = cv_to_d3d(l_mat, false);
  k_icon.size2d_ = l_mat.size();
  return true;
}

image_loader::~image_loader() = default;
}  // namespace doodle
