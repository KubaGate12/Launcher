/*
 * Copyright 2020 Petr Mrázek
 *
 * This source is subject to the Microsoft Permissive License (MS-PL).
 * Please see the COPYING.md file for more information.
 */

#pragma once

#include <QString>
#include <map>
#include <set>
#include <QStringList>
#include "tasks/Task.h"

#include "Path.h"

namespace mojang_files {

using Hash = QString;
extern const Hash empty_hash;

enum class Compression {
    Raw,
    Lzma,
    Unknown
};


struct FileSource
{
    Compression compression = Compression::Unknown;
    Hash hash;
    QString url;
    std::size_t size = 0;
    void upgrade(const FileSource & other) {
        if(compression == Compression::Unknown || other.size < size) {
            *this = other;
        }
    }
    bool isBad() const {
        return compression == Compression::Unknown;
    }
};

struct File
{
    Hash hash;
    bool executable;
    std::uint64_t size = 0;
};

struct Package {
    static Package fromInspectedFolder(const QString &folderPath);
    static Package fromManifestFile(const QString &path);
    static Package fromManifestContents(const QByteArray& contents);

    explicit operator bool() const
    {
        return valid;
    }
    void addFolder(Path folder);
    void addFile(const Path & path, const File & file);
    void addLink(const Path & path, const Path & target);
    void addSource(const Hash & rawHash, const FileSource & source);

    std::map<Hash, FileSource> sources;
    bool valid = true;
    std::set<Path> folders;
    std::map<Path, File> files;
    std::map<Path, Path> symlinks;
};

struct FileDownload : FileSource
{
    FileDownload(const FileSource& source, bool executable) {
        static_cast<FileSource &> (*this) = source;
        this->executable = executable;
    }
    bool executable = false;
};

struct UpdateOperations {
    static UpdateOperations resolve(const Package & from, const Package & to);
    bool valid = false;
    std::vector<Path> deletes;
    std::vector<Path> rmdirs;
    std::vector<Path> mkdirs;
    std::map<Path, FileDownload> downloads;
    std::map<Path, Path> mklinks;
    std::map<Path, bool> executable_fixes;

    bool empty() const {
        if(!valid) {
            return true;
        }
        return
            deletes.empty() &&
            rmdirs.empty() &&
            mkdirs.empty() &&
            downloads.empty() &&
            mklinks.empty() &&
            executable_fixes.empty();
    }
};

}
