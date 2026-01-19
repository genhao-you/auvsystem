#pragma once

#include <QObject>
#include <QString>
class DocumentModel  : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
	Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
	Q_PROPERTY(bool modified READ isModified  NOTIFY modifiedChanged)

public:
	DocumentModel(QObject* parent);
	~DocumentModel();
public:
	QString filePath() const;
	QString content() const;
	bool isModified() const;

public slots:
	bool load(const QString& filePath);
	bool save();
	bool saveAs(const QString& filePath);
	void setContent(const QString& content);
	void setFilePath(const QString& filePath);
	void clear();
signals:
	void filePathChanged(const QString& filePath);
	void contentChanged(const QString& content);
	void modifiedChanged(bool modified);
	void errorOccurred(const QString& message);
private:
	QString  m_sfilePath;
	QString  m_scontent;
	bool m_bmodified;

};
