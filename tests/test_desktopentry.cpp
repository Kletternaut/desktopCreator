#include <QtTest>
#include <QTemporaryFile>
#include <QDir>

#include "desktopentry.h"

/**
 * @brief Testklasse für DesktopEntry
 *
 * Testet:
 *  - loadFromFile() mit validen und invaliden Dateien
 *  - toDesktopFormat() als Roundtrip-Test
 *  - validate() mit fehlenden Pflichtfeldern
 *  - Sprachvarianten (Lokalisierung)
 *  - Custom X-* Felder
 */
class TestDesktopEntry : public QObject
{
    Q_OBJECT

private slots:

    // ── loadFromFile / parseFromString ────────────────────────────────────────

    void testParseMinimalApplication()
    {
        DesktopEntry e;
        const QString content =
            "[Desktop Entry]\n"
            "Version=1.5\n"
            "Type=Application\n"
            "Name=TestApp\n"
            "Exec=/usr/bin/testapp\n"
            "Icon=testapp\n";

        QVERIFY(e.parseFromString(content));
        QCOMPARE(e.type(), DesktopEntry::Type::Application);
        QCOMPARE(e.name(), QString("TestApp"));
        QCOMPARE(e.exec(), QString("/usr/bin/testapp"));
        QCOMPARE(e.icon(), QString("testapp"));
        QCOMPARE(e.version(), QString("1.5"));
    }

    void testParseLink()
    {
        DesktopEntry e;
        const QString content =
            "[Desktop Entry]\n"
            "Type=Link\n"
            "Name=GitHub\n"
            "URL=https://github.com\n";

        QVERIFY(e.parseFromString(content));
        QCOMPARE(e.type(), DesktopEntry::Type::Link);
        QCOMPARE(e.url(), QString("https://github.com"));
    }

    void testParseDirectory()
    {
        DesktopEntry e;
        const QString content =
            "[Desktop Entry]\n"
            "Type=Directory\n"
            "Name=Mein Ordner\n";

        QVERIFY(e.parseFromString(content));
        QCOMPARE(e.type(), DesktopEntry::Type::Directory);
    }

    void testParseBooleanFields()
    {
        DesktopEntry e;
        const QString content =
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=Test\n"
            "Terminal=true\n"
            "NoDisplay=true\n"
            "Hidden=false\n"
            "StartupNotify=true\n";

        QVERIFY(e.parseFromString(content));
        QVERIFY(e.terminal());
        QVERIFY(e.noDisplay());
        QVERIFY(!e.hidden());
        QVERIFY(e.startupNotify());
    }

    void testParseListFields()
    {
        DesktopEntry e;
        const QString content =
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=Test\n"
            "Categories=Audio;Video;Network;\n"
            "Keywords=music;video;\n"
            "MimeType=audio/mp3;video/mp4;\n";

        QVERIFY(e.parseFromString(content));
        QCOMPARE(e.categories(), QStringList({ "Audio", "Video", "Network" }));
        QCOMPARE(e.keywords(),   QStringList({ "music", "video"  }));
        QCOMPARE(e.mimeTypes(),  QStringList({ "audio/mp3", "video/mp4" }));
    }

    void testParseLocalizedName()
    {
        DesktopEntry e;
        const QString content =
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=MyApp\n"
            "Name[de]=MeineApp\n"
            "Name[fr]=MonApp\n";

        QVERIFY(e.parseFromString(content));
        QCOMPARE(e.name(), QString("MyApp"));
        QCOMPARE(e.getLocalized("Name", "de"), QString("MeineApp"));
        QCOMPARE(e.getLocalized("Name", "fr"), QString("MonApp"));
        QCOMPARE(e.getLocalized("Name", "xx"), QString());
    }

    void testParseActions()
    {
        DesktopEntry e;
        const QString content =
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=Test\n"
            "Actions=NewWindow;OpenFile;\n"
            "\n"
            "[Desktop Action NewWindow]\n"
            "Name=Neues Fenster\n"
            "Exec=/usr/bin/test --new-window\n"
            "\n"
            "[Desktop Action OpenFile]\n"
            "Name=Datei öffnen\n"
            "Exec=/usr/bin/test --open %f\n";

        QVERIFY(e.parseFromString(content));
        QCOMPARE(e.actions().size(), 2);
        QCOMPARE(e.actions().at(0).id,   QString("NewWindow"));
        QCOMPARE(e.actions().at(0).name, QString("Neues Fenster"));
        QCOMPARE(e.actions().at(1).id,   QString("OpenFile"));
    }

    void testParseCustomFields()
    {
        DesktopEntry e;
        const QString content =
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=Test\n"
            "X-GNOME-Autostart-enabled=true\n"
            "X-Custom-Field=hello\n";

        QVERIFY(e.parseFromString(content));
        QCOMPARE(e.customField("X-GNOME-Autostart-enabled"), QString("true"));
        QCOMPARE(e.customField("X-Custom-Field"), QString("hello"));
    }

    void testLoadFromNonExistentFile()
    {
        DesktopEntry e;
        QVERIFY(!e.loadFromFile("/tmp/does_not_exist_xyz123.desktop"));
    }

    void testLoadAndSaveRoundtrip()
    {
        QTemporaryFile tmpFile;
        tmpFile.setFileTemplate(QDir::tempPath() + "/test_XXXXXX.desktop");
        QVERIFY(tmpFile.open());
        tmpFile.close();

        const QString content =
            "[Desktop Entry]\n"
            "Version=1.5\n"
            "Type=Application\n"
            "Name=RoundtripTest\n"
            "GenericName=Test Application\n"
            "Comment=Test comment\n"
            "Icon=testicon\n"
            "Exec=/usr/bin/test %U\n"
            "Terminal=false\n"
            "Categories=Development;Utility;\n"
            "Keywords=test;roundtrip;\n";

        DesktopEntry e1;
        QVERIFY(e1.parseFromString(content));
        QVERIFY(e1.saveToFile(tmpFile.fileName()));

        DesktopEntry e2;
        QVERIFY(e2.loadFromFile(tmpFile.fileName()));

        QCOMPARE(e2.name(),        e1.name());
        QCOMPARE(e2.exec(),        e1.exec());
        QCOMPARE(e2.icon(),        e1.icon());
        QCOMPARE(e2.comment(),     e1.comment());
        QCOMPARE(e2.genericName(), e1.genericName());
        QCOMPARE(e2.categories(),  e1.categories());
        QCOMPARE(e2.keywords(),    e1.keywords());
        QCOMPARE(e2.terminal(),    e1.terminal());
    }

    // ── toDesktopFormat ───────────────────────────────────────────────────────

    void testToDesktopFormatContainsMandatoryFields()
    {
        DesktopEntry e;
        e.setType(DesktopEntry::Type::Application);
        e.setName("MyApp");
        e.setExec("/usr/bin/myapp");
        e.setIcon("myapp");

        const QString fmt = e.toDesktopFormat();
        QVERIFY(fmt.contains("[Desktop Entry]"));
        QVERIFY(fmt.contains("Type=Application"));
        QVERIFY(fmt.contains("Name=MyApp"));
        QVERIFY(fmt.contains("Exec=/usr/bin/myapp"));
        QVERIFY(fmt.contains("Icon=myapp"));
    }

    void testToDesktopFormatSemicolonList()
    {
        DesktopEntry e;
        e.setType(DesktopEntry::Type::Application);
        e.setName("Test");
        e.setCategories({ "Audio", "Video" });

        const QString fmt = e.toDesktopFormat();
        QVERIFY(fmt.contains("Categories=Audio;Video;"));
    }

    void testToDesktopFormatLocalizedName()
    {
        DesktopEntry e;
        e.setType(DesktopEntry::Type::Application);
        e.setName("MyApp");
        e.setLocalized("Name", "de", "MeineApp");

        const QString fmt = e.toDesktopFormat();
        QVERIFY(fmt.contains("Name[de]=MeineApp"));
    }

    void testToDesktopFormatActions()
    {
        DesktopEntry e;
        e.setType(DesktopEntry::Type::Application);
        e.setName("Test");

        DesktopEntry::Action a;
        a.id   = "new-window";
        a.name = "Neues Fenster";
        a.exec = "/usr/bin/test --new";
        e.setActions({ a });

        const QString fmt = e.toDesktopFormat();
        QVERIFY(fmt.contains("Actions=new-window;"));
        QVERIFY(fmt.contains("[Desktop Action new-window]"));
        QVERIFY(fmt.contains("Name=Neues Fenster"));
    }

    // ── validate ─────────────────────────────────────────────────────────────

    void testValidateApplicationMissingName()
    {
        DesktopEntry e;
        e.setType(DesktopEntry::Type::Application);
        // Name leer lassen
        e.setExec("/usr/bin/test");

        const auto result = e.validate();
        QVERIFY(!result.valid);
        QVERIFY(!result.errors.isEmpty());
    }

    void testValidateApplicationMissingExec()
    {
        DesktopEntry e;
        e.setType(DesktopEntry::Type::Application);
        e.setName("Test");
        // Exec leer lassen

        const auto result = e.validate();
        QVERIFY(!result.valid);
        bool hasExecError = false;
        for (const auto &err : result.errors)
            if (err.contains("Exec")) { hasExecError = true; break; }
        QVERIFY(hasExecError);
    }

    void testValidateLinkMissingUrl()
    {
        DesktopEntry e;
        e.setType(DesktopEntry::Type::Link);
        e.setName("GitHub");
        // URL leer lassen

        const auto result = e.validate();
        QVERIFY(!result.valid);
        bool hasUrlError = false;
        for (const auto &err : result.errors)
            if (err.contains("URL")) { hasUrlError = true; break; }
        QVERIFY(hasUrlError);
    }

    void testValidateValidApplication()
    {
        DesktopEntry e;
        e.setType(DesktopEntry::Type::Application);
        e.setName("TestApp");
        e.setExec("/usr/bin/testapp");
        e.setIcon("testapp");

        const auto result = e.validate();
        QVERIFY(result.valid);
        QVERIFY(result.errors.isEmpty());
    }

    void testValidateWarnsMissingIcon()
    {
        DesktopEntry e;
        e.setType(DesktopEntry::Type::Application);
        e.setName("TestApp");
        e.setExec("/usr/bin/testapp");
        // Kein Icon

        const auto result = e.validate();
        QVERIFY(result.valid);  // Fehler: nein; Warnungen: ja
        QVERIFY(!result.warnings.isEmpty());
    }

    // ── reset ─────────────────────────────────────────────────────────────────

    void testReset()
    {
        DesktopEntry e;
        e.setName("Test");
        e.setExec("/usr/bin/foo");
        e.setTerminal(true);

        e.reset();

        QVERIFY(e.name().isEmpty());
        QVERIFY(e.exec().isEmpty());
        QVERIFY(!e.terminal());
        QCOMPARE(e.type(), DesktopEntry::Type::Application);
    }

    // ── typeToString / stringToType ──────────────────────────────────────────

    void testTypeConversion()
    {
        QCOMPARE(DesktopEntry::typeToString(DesktopEntry::Type::Application), QString("Application"));
        QCOMPARE(DesktopEntry::typeToString(DesktopEntry::Type::Link),        QString("Link"));
        QCOMPARE(DesktopEntry::typeToString(DesktopEntry::Type::Directory),   QString("Directory"));

        QCOMPARE(DesktopEntry::stringToType("Application"), DesktopEntry::Type::Application);
        QCOMPARE(DesktopEntry::stringToType("Link"),        DesktopEntry::Type::Link);
        QCOMPARE(DesktopEntry::stringToType("Directory"),   DesktopEntry::Type::Directory);
        QCOMPARE(DesktopEntry::stringToType("Unknown"),     DesktopEntry::Type::Application);
    }
};

QTEST_MAIN(TestDesktopEntry)
#include "test_desktopentry.moc"
