/*
    Copyright 2017 Henrik Fehlauer <rkflx@lab12.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ksqueezedtextlabelautotest.h"

#include <KSqueezedTextLabel>

#include <QTest>

Q_DECLARE_METATYPE(Qt::TextElideMode)

namespace
{

KSqueezedTextLabel* createLabel(const QString &text = QStringLiteral("Squeeze me"))
{
    KSqueezedTextLabel *label = new KSqueezedTextLabel(text, 0);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    label->show();
    return label;
}

void squeezeLabel(KSqueezedTextLabel *label, const int pixels = 1)
{
    label->resize(label->size().width() - pixels, label->size().height());
}


} // namespace

void KSqueezedTextLabelAutotest::testEmptyText()
{
    const QScopedPointer<KSqueezedTextLabel> label(createLabel(QString()));

    QVERIFY(label->text().isEmpty());
    QVERIFY(label->fullText().isEmpty());
    QVERIFY(label->toolTip().isEmpty());
    QVERIFY(!label->isSqueezed());
    QCOMPARE(label->width(), 0);
}

void KSqueezedTextLabelAutotest::testElisionOnResize_data()
{
    QTest::addColumn<QString>("text");

    QTest::newRow("whitespace text") << "             ";
    QTest::newRow("normal text") << "Squeeze me";
    QTest::newRow("rich text") << "<h1>Squeeze me!</h1>";
//     QTest::newRow("multiline text") << "Squeeze me,\nand\nme too.";
//     QTest::newRow("multiline rich text") << "<i>Squeeze</i> me,\n<b>and\nme</b> too.";
}

void KSqueezedTextLabelAutotest::testElisionOnResize()
{
    QFETCH(QString, text);

    const QScopedPointer<KSqueezedTextLabel> label(createLabel(text));

    QVERIFY(label->text() == text);
    QVERIFY(label->fullText() == text);
    QVERIFY(label->toolTip().isEmpty());
    QVERIFY(!label->isSqueezed());

    squeezeLabel(label.data());

    QVERIFY(label->text() != text);
    QVERIFY(label->fullText() == text);
    QVERIFY(label->toolTip() == text);
    QVERIFY(label->isSqueezed());

    squeezeLabel(label.data(), -1);

    QVERIFY(label->text() == text);
    QVERIFY(label->fullText() == text);
    QVERIFY(label->toolTip().isEmpty());
    QVERIFY(!label->isSqueezed());
}

void KSqueezedTextLabelAutotest::testElisionOnTextUpdate()
{
    const QScopedPointer<KSqueezedTextLabel> label(createLabel());

    QVERIFY(!label->isSqueezed());

    label->setText(label->text() + QStringLiteral("!!!"));

    QVERIFY(label->isSqueezed());
}

void KSqueezedTextLabelAutotest::testElideMode_data()
{
    QTest::addColumn<Qt::TextElideMode>("mode");
    QTest::addColumn<QChar>("prefix");
    QTest::addColumn<QChar>("infix");
    QTest::addColumn<QChar>("postfix");

    const QChar ellipsisChar(0x2026);
    const QChar a(QLatin1Char('a')), b(QLatin1Char('b')), c(QLatin1Char('c'));

    QTest::newRow("ElideLeft") << Qt::ElideLeft << ellipsisChar << b << c;
    QTest::newRow("ElideRight") << Qt::ElideRight << a << b << ellipsisChar;
    QTest::newRow("ElideMiddle") << Qt::ElideMiddle << a << ellipsisChar << c;
    QTest::newRow("ElideNone") << Qt::ElideNone << a << b << c;
}

void KSqueezedTextLabelAutotest::testElideMode()
{
    QFETCH(Qt::TextElideMode, mode);
    QFETCH(QChar, prefix);
    QFETCH(QChar, infix);
    QFETCH(QChar, postfix);

    // "abc" is not sufficient, because ellipsis might be wider than a single char
    const QScopedPointer<KSqueezedTextLabel> label(createLabel(QStringLiteral("aaabbbccc")));
    label->setTextElideMode(mode);
    squeezeLabel(label.data());
    const int infixPos = label->text().indexOf(infix);

    QVERIFY(label->isSqueezed() || mode == Qt::ElideNone);
    QVERIFY(label->text().startsWith(prefix));
    QVERIFY(0 < infixPos && infixPos < label->text().length()-1);
    QVERIFY(label->text().endsWith(postfix));
}

void KSqueezedTextLabelAutotest::testSizeHints()
{
    const QScopedPointer<KSqueezedTextLabel> label(createLabel(QString()));

    QVERIFY(!label->isSqueezed());
    QCOMPARE(label->size().width(), 0);
    QCOMPARE(label->minimumSizeHint().width(), -1);
    QCOMPARE(label->sizeHint().width(), 0);

    label->adjustSize();

    QVERIFY(!label->isSqueezed());
    QCOMPARE(label->size().width(), 0);
    QCOMPARE(label->minimumSizeHint().width(), -1);
    QCOMPARE(label->sizeHint().width(), 0);

    const QString text = QStringLiteral("Squeeze me");
    const int labelWidth = label->fontMetrics().width(text);  // no chrome set
    label->setText(text);

    QVERIFY(label->isSqueezed());
    QCOMPARE(label->size().width(), 0);
    QCOMPARE(label->minimumSizeHint().width(), -1);
    QCOMPARE(label->sizeHint().width(), labelWidth);

    label->adjustSize();

    QVERIFY(!label->isSqueezed());
    QCOMPARE(label->size().width(), labelWidth);
    QCOMPARE(label->minimumSizeHint().width(), -1);
    QCOMPARE(label->sizeHint().width(), labelWidth);

    const int indent = 40;
    label->setIndent(indent);
    label->adjustSize();

    QVERIFY(!label->isSqueezed());
    QCOMPARE(label->size().width(), labelWidth + indent);
    QCOMPARE(label->minimumSizeHint().width(), -1);
    QCOMPARE(label->sizeHint().width(), labelWidth + indent);
}

void KSqueezedTextLabelAutotest::testClearing()
{
    const QScopedPointer<KSqueezedTextLabel> label(createLabel());
    squeezeLabel(label.data());

    QVERIFY(label->isSqueezed());

    label->clear();

    QVERIFY(label->text().isEmpty());
    QVERIFY(label->fullText().isEmpty());
    QEXPECT_FAIL("", "To fix: Reset tooltip in clear()", Continue);
    QVERIFY(label->toolTip().isEmpty());
    QVERIFY(!label->isSqueezed());
}

void KSqueezedTextLabelAutotest::testChrome_data()
{
    const QFontMetrics fm(KSqueezedTextLabel().fontMetrics());
    const int xWidth = fm.width(QLatin1Char('x')) / 2;

    QTest::addColumn<QString>("attribute");
    QTest::addColumn<int>("amount");
    QTest::addColumn<int>("widthDifference");

    QTest::newRow("indent") << "indent" << 20 << 20;
    QTest::newRow("margin") << "margin" << 20 << 40;
    QTest::newRow("frame") << "lineWidth" << 20 << 40 + xWidth;
}

void KSqueezedTextLabelAutotest::testChrome()
{
    QFETCH(QString, attribute);
    QFETCH(int, amount);
    QFETCH(int, widthDifference);

    const QScopedPointer<KSqueezedTextLabel> label(createLabel());
    label->setFrameStyle(QFrame::Box);
    label->setLineWidth(0);
    const int oldWidth = label->width();

    QVERIFY(!label->isSqueezed());

    label->setProperty(attribute.toLatin1().data(), amount);

    QVERIFY(label->isSqueezed());

    label->adjustSize();

    QVERIFY(!label->isSqueezed());
    QCOMPARE(label->width(), oldWidth + widthDifference);
}

QTEST_MAIN(KSqueezedTextLabelAutotest)


// TODO
// currently untested, may need clarification/definition of correct behaviour:
// - multiline text
// - setWordWrap
// - setAlignment
// - setToolTip
// - context menu
// - Qt::TextInteractionFlags, keyboard/mouse interaction
// - linkHovered/linkActivated
// - setPixmap
