#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/config-file.h>

#include <QMainWindow>
#include <QDockWidget>
#include <QWidget>
#include <QLabel>
#include <QSpinBox>
#include <QString>
#include <QStringList>
#include <QScrollArea>
#include <QGridLayout>
#include <QEvent>
#include <QThread>
#include <QTimer>
#include <QComboBox>
#include <QAction>
#include <QMessageBox>

#define ConfigSection "obs-auto-stop-streaming"
#define TitleLabelHeight (int)(22 * QWidget::devicePixelRatioF())

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-auto-stop-streraming", "en-US")
OBS_MODULE_AUTHOR("Kagua Kurusu")

const char *obs_module_name(void)
{
	return "Auto stop streaming";
}

const char *obs_module_description(void)
{
	return "Auto stop streaming then after setting time changeing to selected scean";
}

QStringList convert_to_QStringList(char **src)
{
	QStringList result;

	while (*src != NULL) {
		result << *src;
		**src++;
	}

	return result;
}

class AutoStopStreamingWidget : public QDockWidget
{
	int dock_location;
	bool dock_visible;

public:
	AutoStopStreamingWidget(QMainWindow* parent = 0)
		: QDockWidget(parent)
	{
		setWindowTitle(obs_module_text("Title"));
		setFeatures((DockWidgetFeatures)(AllDockWidgetFeatures & ~DockWidgetClosable));

		// save dock location
		connect(this, &QDockWidget::dockLocationChanged, [this](Qt::DockWidgetArea area) {
			dock_location = area;
		});

		timer = new QTimer(this);
		container = new QWidget(this);
		scroll = new QScrollArea(this);
		scroll->move(0, TitleLabelHeight);
		layout = new QGridLayout(container);
		layout->setAlignment(Qt::AlignmentFlag::AlignTop);


		// init widget
		time_spin_box = new QSpinBox(container);
		time_spin_box->setMaximum(INT_MAX / 1000);
		time_spin_box->setSuffix(obs_module_text("Time.Suffix"));
		time_spin_box->setValue(60);
		scenes_combo_box = new QComboBox(container);
		layout->setColumnStretch(0, 1);
		layout->setColumnStretch(1, 1);
		layout->addWidget(new QLabel(obs_module_text("Time.Label")), 0, 0);
		layout->addWidget(time_spin_box, 0, 1);
		layout->addWidget(new QLabel(obs_module_text("Scene.Label")), 1, 0);
		layout->addWidget(scenes_combo_box, 1, 1);

		scroll->setWidgetResizable(true);
		scroll->setWidget(container);

		setLayout(layout);

		resize(200, 400);
	}

	bool event(QEvent *event) override
	{
		if (event->type() == QEvent::Resize)
		{
			scroll->resize(width(), height() - TitleLabelHeight);
		}

		return QDockWidget::event(event);
	}

	void SaveConfig()
	{
		config_set_int(obs_frontend_get_global_config(), ConfigSection, "Time", time_spin_box->value());
		config_set_string(obs_frontend_get_global_config(), ConfigSection, "Scene", scenes_combo_box->currentText().toUtf8().data());
		config_set_int(obs_frontend_get_global_config(), ConfigSection, "DockLocation", dock_location);
		config_set_bool(obs_frontend_get_global_config(), ConfigSection, "DockVisible", dock_visible);
	}

	void LoadConfig()
	{
		auto time = config_get_int(obs_frontend_get_global_config(), ConfigSection, "Time");
		auto scene = config_get_string(obs_frontend_get_global_config(), ConfigSection, "Scene");

		if (config_has_user_value(obs_frontend_get_global_config(), ConfigSection, "Time")) {
			time_spin_box->setValue(time);
		}

		if (config_has_user_value(obs_frontend_get_global_config(), ConfigSection, "Scene")) {
			scenes_combo_box->setCurrentText(scene);
		}
	}

	void setSceneComboBox()
	{
		QStringList scenes = convert_to_QStringList(obs_frontend_get_scene_names());

		scenes_combo_box->clear();
		scenes_combo_box->addItems(scenes);
	}

	void startTimer()
	{
		if (obs_frontend_streaming_active() == true) {
			if (QString::compare(
				scenes_combo_box->currentText(),
				(QString)obs_source_get_name(obs_frontend_get_current_scene()),
				Qt::CaseSensitive
			) == 0) {
				timer->setSingleShot(true);
				connect(timer, &QTimer::timeout, this, [this](){
					obs_frontend_streaming_stop();
					stopTimer();
				});
				timer->start(time_spin_box->value() * 1000);
			}
			else {
				stopTimer();
			}
		}
	}

	void stopTimer()
	{
		if (timer->isActive()) {
			timer->stop();
		}
	}

	void visibleToggled(bool visible)
	{
		dock_visible = visible;
		return;
	}

private:
	QWidget* container = 0;
	QScrollArea* scroll = 0;
	QGridLayout* layout = 0;
	QSpinBox* time_spin_box = 0;
	QComboBox* scenes_combo_box = 0;
	QTimer* timer = 0;
};

void obsstudio_auto_stop_streaming_frontend_event_callback(enum obs_frontend_event event, void *private_data)
{
	auto dock = (AutoStopStreamingWidget*)private_data;

	switch (event) {
		case OBS_FRONTEND_EVENT_SCENE_CHANGED:
			dock->startTimer();
			break;
		case OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED:
		case OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED:
		case OBS_FRONTEND_EVENT_PROFILE_CHANGED:
			dock->setSceneComboBox();
			break;
		case OBS_FRONTEND_EVENT_EXIT:
			dock->stopTimer();
			dock->SaveConfig();
			break;
		case OBS_FRONTEND_EVENT_FINISHED_LOADING:
			dock->setSceneComboBox();
			dock->LoadConfig();
			break;
	}
}

bool obs_module_load(void)
{
	auto mainWin = (QMainWindow*)obs_frontend_get_main_window();
	auto dock = new AutoStopStreamingWidget(mainWin);
	auto action = (QAction*)obs_frontend_add_dock(dock);
	auto dock_location = config_get_int(obs_frontend_get_global_config(), ConfigSection, "DockLocation");
	auto visible = config_get_bool(obs_frontend_get_global_config(), ConfigSection, "DockVisible");

	QObject::connect(action, &QAction::toggled, dock, &AutoStopStreamingWidget::visibleToggled);

	mainWin->removeDockWidget(dock);
	if (!config_has_user_value(obs_frontend_get_global_config(), ConfigSection, "DockLocation")){
		dock_location = Qt::DockWidgetArea::LeftDockWidgetArea;
	}

	if (!config_has_user_value(obs_frontend_get_global_config(), ConfigSection, "DockVisible")) {
		visible = true;
	}

	mainWin->addDockWidget((Qt::DockWidgetArea)dock_location, dock);
	if (visible) {
		dock->setVisible(true);
		action->setChecked(true);
	}
	else {
		dock->setVisible(false);
		action->setChecked(false);
	}

	obs_frontend_add_event_callback(obsstudio_auto_stop_streaming_frontend_event_callback, (void*)dock);
	return true;
}
