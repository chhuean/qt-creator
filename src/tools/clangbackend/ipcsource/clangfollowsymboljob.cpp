/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "clangfollowsymboljob.h"

#include <clangbackendipc/clangbackendipcdebugutils.h>
#include <clangbackendipc/followsymbolmessage.h>
#include <clangbackendipc/clangcodemodelclientinterface.h>

#include <utils/qtcassert.h>

namespace ClangBackEnd {

static FollowSymbolJob::AsyncResult runAsyncHelperFollow(const TranslationUnit &translationUnit,
                                                         quint32 line,
                                                         quint32 column,
                                                         const QVector<Utf8String> &dependentFiles,
                                                         bool resolveTarget)
{
    TIME_SCOPE_DURATION("FollowSymbolJobRunner");

    return FollowSymbolResult();
}

IAsyncJob::AsyncPrepareResult FollowSymbolJob::prepareAsyncRun()
{
    const JobRequest jobRequest = context().jobRequest;
    QTC_ASSERT(jobRequest.type == JobRequest::Type::FollowSymbol,
               return AsyncPrepareResult());

    try {
        m_pinnedDocument = context().documentForJobRequest();
        m_pinnedFileContainer = m_pinnedDocument.fileContainer();

        const TranslationUnit translationUnit
                = m_pinnedDocument.translationUnit(jobRequest.preferredTranslationUnit);
        const quint32 line = jobRequest.line;
        const quint32 column = jobRequest.column;
        const QVector<Utf8String> &dependentFiles = jobRequest.dependentFiles;
        const bool resolveTarget = jobRequest.resolveTarget;
        setRunner([translationUnit, line, column, dependentFiles, resolveTarget]() {
            return runAsyncHelperFollow(translationUnit, line, column, dependentFiles, resolveTarget);
        });
        return AsyncPrepareResult{translationUnit.id()};

    } catch (const std::exception &exception) {
        qWarning() << "Error in FollowSymbolJob::prepareAsyncRun:" << exception.what();
        return AsyncPrepareResult();
    }
}

void FollowSymbolJob::finalizeAsyncRun()
{
    if (!context().isOutdated()) {
        const AsyncResult result = asyncResult();

        const FollowSymbolMessage message(m_pinnedFileContainer,
                                          result.m_range,
                                          result.m_failedToFollow,
                                          context().jobRequest.ticketNumber);
        context().client->followSymbol(message);
    }
}

} // namespace ClangBackEnd
