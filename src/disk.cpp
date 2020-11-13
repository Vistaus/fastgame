#include "disk.hpp"
#include <glibmm/i18n.h>
#include <filesystem>
#include <fstream>
#include <string>
#include "util.hpp"

namespace fs = std::filesystem;

Disk::Disk(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder) : Gtk::Grid(cobject) {
  // loading glade widgets

  builder->get_widget("device", device);
  builder->get_widget("scheduler", scheduler);
  builder->get_widget("enable_realtime_priority", enable_realtime_priority);
  builder->get_widget("add_random", add_random);

  readahead = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("readahead"));
  nr_requests = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(builder->get_object("nr_requests"));

  // initializing widgets

  for (const auto& entry : std::filesystem::directory_iterator("/sys/block")) {
    device->append(entry.path().string());
  }

  device->signal_changed().connect([=]() {
    init_scheduler();

    nr_requests->set_value(std::stoi(util::read_system_setting(device->get_active_text() + "/queue/nr_requests")[0]));

    readahead->set_value(std::stoi(util::read_system_setting(device->get_active_text() + "/queue/read_ahead_kb")[0]));

    add_random->set_active(
        static_cast<bool>(std::stoi(util::read_system_setting(device->get_active_text() + "/queue/add_random")[0])));
  });

  device->set_active(0);
}

Disk::~Disk() {
  util::debug(log_tag + "destroyed");
}

auto Disk::add_to_stack(Gtk::Stack* stack) -> Disk* {
  auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/fastgame/ui/disk.glade");

  Disk* ui = nullptr;

  builder->get_widget_derived("widgets_grid", ui);

  stack->add(*ui, "disk", _("Storage Device"));

  return ui;
}

void Disk::init_scheduler() {
  auto scheduler_list = util::read_system_setting(device->get_active_text() + "/queue/scheduler");

  auto scheduler_value = util::get_selected_value(scheduler_list);

  scheduler->remove_all();

  for (auto& value : scheduler_list) {
    if (value.find('[') != std::string::npos) {
      value = value.erase(0, 1).erase(value.size() - 1, 1);  // removing the [] characters
    }

    scheduler->append(value);
  }

  scheduler->set_active_text(scheduler_value);
}

auto Disk::get_device() -> std::string {
  return device->get_active_text();
}

void Disk::set_device(const std::string& value) {
  device->set_active_text(value);
}

auto Disk::get_scheduler() -> std::string {
  return scheduler->get_active_text();
}

void Disk::set_scheduler(const std::string& value) {
  scheduler->set_active_text(value);
}

auto Disk::get_readahead() -> int {
  return static_cast<int>(readahead->get_value());
}

void Disk::set_readahead(const int& value) {
  readahead->set_value(value);
}

auto Disk::get_nr_requests() -> int {
  return static_cast<int>(nr_requests->get_value());
}

void Disk::set_nr_requests(const int& value) {
  nr_requests->set_value(value);
}

auto Disk::get_enable_realtime_priority() -> bool {
  return enable_realtime_priority->get_active();
}

void Disk::set_enable_realtime_priority(const bool& value) {
  enable_realtime_priority->set_active(value);
}

auto Disk::get_enable_add_random() -> bool {
  return add_random->get_active();
}

void Disk::set_enable_add_random(const bool& value) {
  add_random->set_active(value);
}