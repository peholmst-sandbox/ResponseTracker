#include <QtTest>

#include "model.h"

using namespace Base::Model;

class ModelTest : public QObject {
    Q_OBJECT
  private slots:
    void property_state_no_initial_value();
    void property_state_initial_value();
    void property_state_set_value_operator();
    void property_state_set_value_method();
    void property_state_set_value_different_scope();
    void property_state_clear();
    void property_equality_both_empty();
    void property_equality_one_empty();
    void property_equality_different_values();
    void property_equality_same_values();
    void property_event_value_changed();
};

class ValueChangeListener : Base::Event::EventHandler<ValueChangeListener> {
  public:
    QString lastNewValue;

    void onValueChanged(Property<QString>& sender, const QString& newValue) {
        lastNewValue = newValue;
    }
};

void ModelTest::property_state_no_initial_value() {
    Property<QString> p;
    QVERIFY(p.isEmpty());
    QVERIFY(!p.hasValue());
    QVERIFY_EXCEPTION_THROWN(p.value(), std::exception);
}

void ModelTest::property_state_initial_value() {
    Property<QString> p("hello world");
    QVERIFY(!p.isEmpty());
    QVERIFY(p.hasValue());
    QCOMPARE(p.value(), "hello world");
}

void ModelTest::property_state_set_value_operator() {
    Property<QString> p;
    p = "hello world";
    QCOMPARE(p.value(), "hello world");
}

void ModelTest::property_state_set_value_method() {
    Property<QString> p;
    p.setValue("hello world");
    QCOMPARE(p.value(), "hello world");
}

void ModelTest::property_state_set_value_different_scope() {
    Property<QString> p;
    {
        QString s("hello world");
        p = s;
    }
    QCOMPARE(p.value(), "hello world");
}

void ModelTest::property_state_clear() {
    Property<QString> p("hello world");
    p.clear();
    QVERIFY(p.isEmpty());
}

void ModelTest::property_equality_both_empty() {
    Property<QString> p1, p2;
    QVERIFY(p1 == p2);
}

void ModelTest::property_equality_one_empty() {
    Property<QString> p1("hello"), p2;
    QVERIFY(p1 != p2);
}

void ModelTest::property_equality_different_values() {
    Property<QString> p1("hello"), p2("world");
    QVERIFY(p1 != p2);
}

void ModelTest::property_equality_same_values() {
    Property<QString> p1("hello"), p2("hello");
    QVERIFY(p1 == p2);
}

void ModelTest::property_event_value_changed() { Property<QString> p; }

QTEST_APPLESS_MAIN(ModelTest);

#include "tst_modeltest.moc"
