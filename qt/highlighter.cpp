#include "highlighter.h"
#include <QFont>

CodeHighlighter::CodeHighlighter(QTextDocument* doc) : QSyntaxHighlighter(doc) {
    QTextCharFormat kw;
    kw.setForeground(QColor("#569CD6"));
    kw.setFontWeight(QFont::Bold);
    for (const QString& w : QStringList{
            "int","float","double","bool","char","void","string",
            "if","else","for","while","do","return","break","continue",
            "class","struct","public","private","new","delete",
            "true","false","nullptr","const","auto","using","namespace",
            "include","define","endl","cout","cin"}) {
        m_rules.push_back({ QRegularExpression("\\b" + w + "\\b"), kw });
    }

    QTextCharFormat pre; pre.setForeground(QColor("#9B9BEB"));
    m_rules.push_back({ QRegularExpression("^\\s*#[^\n]*"), pre });

    QTextCharFormat str; str.setForeground(QColor("#CE9178"));
    m_rules.push_back({ QRegularExpression("\"[^\"]*\""), str });

    QTextCharFormat num; num.setForeground(QColor("#B5CEA8"));
    m_rules.push_back({ QRegularExpression("\\b[0-9]+\\.?[0-9]*\\b"), num });

    QTextCharFormat cmt; cmt.setForeground(QColor("#6A9955")); cmt.setFontItalic(true);
    m_rules.push_back({ QRegularExpression("//[^\n]*"), cmt });

    m_mlFmt.setForeground(QColor("#6A9955")); m_mlFmt.setFontItalic(true);
}

void CodeHighlighter::highlightBlock(const QString& text) {
    for (auto& r : m_rules) {
        auto it = r.pattern.globalMatch(text);
        while (it.hasNext()) {
            auto m = it.next();
            setFormat(m.capturedStart(), m.capturedLength(), r.format);
        }
    }
    setCurrentBlockState(0);
    QRegularExpression s("/\\*"), e("\\*/");
    int start = (previousBlockState() == 1) ? 0 : text.indexOf(s);
    while (start >= 0) {
        auto m = e.match(text, start);
        int end = m.hasMatch() ? m.capturedEnd() : text.length();
        setCurrentBlockState(m.hasMatch() ? 0 : 1);
        setFormat(start, end - start, m_mlFmt);
        start = m.hasMatch() ? text.indexOf(s, end) : -1;
    }
}
