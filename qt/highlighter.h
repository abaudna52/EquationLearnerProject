#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <vector>

// =============================================================================
// CodeHighlighter — подсветка синтаксиса C++ для вкладки «Код C++»
// =============================================================================

class CodeHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit CodeHighlighter(QTextDocument* doc);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct Rule { QRegularExpression pattern; QTextCharFormat format; };
    std::vector<Rule> m_rules;
    QTextCharFormat   m_mlFmt;   // многострочный комментарий /* ... */
};
