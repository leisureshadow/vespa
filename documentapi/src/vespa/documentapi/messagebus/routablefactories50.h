// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include "routablefactories42.h"
#include "iroutablefactory.h"
#include <vespa/documentapi/messagebus/messages/putdocumentmessage.h>
#include <vespa/documentapi/messagebus/messages/removedocumentmessage.h>
#include <vespa/documentapi/messagebus/messages/updatedocumentmessage.h>

namespace document { class DocumentTypeRepo; }

/**
 * Utility class for invoking setApproxSize on a DocumentMessage with the delta
 * between the read position of a ByteBuffer at construction and destruction
 * time. The assumption being made is that the in-memory footprint of a message
 * is reasonably close to its wire-serialized form.
 */
class ScopedApproxSizeSetter {
public:
    ScopedApproxSizeSetter(documentapi::DocumentMessage& msg,
                           const document::ByteBuffer& buf)
        : _msg(msg),
          _buf(buf),
          _posBefore(_buf.getPos())
    {
    }

    ~ScopedApproxSizeSetter() {
        _msg.setApproxSize(static_cast<uint32_t>(_buf.getPos() - _posBefore));
    }

private:
    documentapi::DocumentMessage& _msg;
    const document::ByteBuffer& _buf;
    const size_t _posBefore;
};

namespace documentapi {

template<typename MessageType, typename FactoryType>
DocumentMessage::UP
decodeMessage(const FactoryType * self, document::ByteBuffer & buf) {
    auto msg = std::make_unique<MessageType>();
    ScopedApproxSizeSetter sizeSetter(*msg, buf);
    self->decodeInto(*msg, buf);
    return msg;
}

/**
 * This class encapsulates all the {@link RoutableFactory} classes needed to implement factories for the document
 * routable. When adding new factories to this class, please KEEP THE THEM ORDERED alphabetically like they are now.
 */
class RoutableFactories50 {
public:
    RoutableFactories50() = delete;

    /**
     * Implements the shared factory logic required for {@link DocumentMessage} objects, and it offers a more
     * convenient interface for implementing {@link RoutableFactory}.
     */
    class DocumentMessageFactory : public IRoutableFactory {
    protected:
        /**
         * This method encodes the given message into the given byte buffer. You are guaranteed to only receive messages of
         * the type that this factory was registered for.
         *
         * This method is NOT exception safe. Return false to signal failure.
         *
         * @param msg The message to encode.
         * @param buf The byte buffer to write to.
         * @return True if the message was encoded.
         */
        virtual bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const = 0;

        /**
         * This method decodes a message from the given byte buffer. You are guaranteed to only receive byte buffers
         * generated by a previous call to {@link #doEncode(DocumentMessage, GrowableByteBuffer)}.
         *
         * This method is NOT exception safe. Return null to signal failure.
         *
         * @param buf The byte buffer to read from.
         * @return The decoded message.
         */
        virtual DocumentMessage::UP doDecode(document::ByteBuffer &buf) const = 0;

    public:
        /**
         * Convenience typedefs.
         */
        typedef std::unique_ptr<IRoutableFactory> UP;
        typedef std::shared_ptr<IRoutableFactory> SP;
        bool encode(const mbus::Routable &obj, vespalib::GrowableByteBuffer &out) const override;
        mbus::Routable::UP decode(document::ByteBuffer &in, const LoadTypeSet& loadTypes) const override;
    };

    /**
     * Implements the shared factory logic required for {@link DocumentReply} objects, and it offers a more
     * convenient interface for implementing {@link RoutableFactory}.
     */
    class DocumentReplyFactory : public IRoutableFactory {
    protected:
        /**
         * This method encodes the given reply into the given byte buffer. You are guaranteed to only receive
         * replies of the type that this factory was registered for.
         *
         * This method is NOT exception safe. Return false to signal failure.
         *
         * @param reply The reply to encode.
         * @param buf The byte buffer to write to.
         * @return True if the message was encoded.
         */
        virtual bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const = 0;

        /**
         * This method decodes a reply from the given byte buffer. You are guaranteed to only receive byte buffers
         * generated by a previous call to {@link #doEncode(DocumentReply, GrowableByteBuffer)}.
         *
         * This method is NOT exception safe. Return null to signal failure.
         *
         * @param buf The byte buffer to read from.
         * @return The decoded reply.
         */
        virtual DocumentReply::UP doDecode(document::ByteBuffer &buf) const = 0;

    public:
        /**
         * Convenience typedefs.
         */
        typedef std::unique_ptr<IRoutableFactory> UP;
        typedef std::shared_ptr<IRoutableFactory> SP;

        bool encode(const mbus::Routable &obj, vespalib::GrowableByteBuffer &out) const override;
        mbus::Routable::UP decode(document::ByteBuffer &in, const LoadTypeSet& loadTypes) const override;
    };

    /**
     * Implements a helper class to do feed message factories.
     */
    class FeedMessageFactory : public DocumentMessageFactory {
    protected:
        void myDecode(FeedMessage &msg, document::ByteBuffer &buf) const;
        void myEncode(const FeedMessage &msg, vespalib::GrowableByteBuffer &buf) const;
    };

    /**
     * Implements a helper class to do feed reply factories.
     */
    class FeedReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
        virtual uint32_t getType() const = 0;
    };

    ////////////////////////////////////////////////////////////////////////////////
    //
    // Factories
    //
    ////////////////////////////////////////////////////////////////////////////////
    class CreateVisitorMessageFactory : public DocumentMessageFactory {
        const document::DocumentTypeRepo &_repo;
    protected:
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    public:
        CreateVisitorMessageFactory(const document::DocumentTypeRepo &r) : _repo(r) {}
    };
    class CreateVisitorReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class DestroyVisitorMessageFactory : public DocumentMessageFactory {
    protected:
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    };
    class DestroyVisitorReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class DocumentListMessageFactory : public DocumentMessageFactory {
        const document::DocumentTypeRepo &_repo;
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    public:
        DocumentListMessageFactory(const document::DocumentTypeRepo &r)
            : _repo(r) {}
    };
    class DocumentListReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class DocumentSummaryMessageFactory : public DocumentMessageFactory {
    protected:
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    };
    class DocumentSummaryReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class EmptyBucketsMessageFactory : public DocumentMessageFactory {
    protected:
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    };
    class EmptyBucketsReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class GetBucketListMessageFactory : public DocumentMessageFactory {
        virtual bool encodeBucketSpace(vespalib::stringref bucketSpace, vespalib::GrowableByteBuffer& buf) const;
        virtual string decodeBucketSpace(document::ByteBuffer&) const;
    protected:
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    };
    class GetBucketListReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class GetBucketStateMessageFactory : public DocumentMessageFactory {
    protected:
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    };
    class GetBucketStateReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class GetDocumentMessageFactory : public DocumentMessageFactory {
    protected:
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    };
    class GetDocumentReplyFactory : public DocumentReplyFactory {
        const document::DocumentTypeRepo &_repo;
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &msg, vespalib::GrowableByteBuffer &buf) const override;
    public:
        GetDocumentReplyFactory(const document::DocumentTypeRepo &r) : _repo(r) {}
    };
    class MapVisitorMessageFactory : public DocumentMessageFactory {
        const document::DocumentTypeRepo &_repo;
    protected:
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    public:
        MapVisitorMessageFactory(const document::DocumentTypeRepo &r) : _repo(r) {}
    };
    class MapVisitorReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class PutDocumentMessageFactory : public DocumentMessageFactory {
    protected:
        const document::DocumentTypeRepo &_repo;
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override {
            return decodeMessage<PutDocumentMessage>(this, buf);
        }

        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    public:
        void decodeInto(PutDocumentMessage & msg, document::ByteBuffer & buf) const;
        PutDocumentMessageFactory(const document::DocumentTypeRepo &r) : _repo(r) {}
    };
    class PutDocumentReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class RemoveDocumentMessageFactory : public DocumentMessageFactory {
    protected:
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override {
            return decodeMessage<RemoveDocumentMessage>(this, buf);
        }

        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    public:
        void decodeInto(RemoveDocumentMessage & msg, document::ByteBuffer & buf) const;
    };
    class RemoveDocumentReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class RemoveLocationMessageFactory : public DocumentMessageFactory {
        const document::DocumentTypeRepo &_repo;
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    public:
        RemoveLocationMessageFactory(const document::DocumentTypeRepo &r) : _repo(r) {}
    };
    class RemoveLocationReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class SearchResultMessageFactory : public DocumentMessageFactory {
    protected:
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    };
    class SearchResultReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class StatBucketMessageFactory : public DocumentMessageFactory {
        virtual bool encodeBucketSpace(vespalib::stringref bucketSpace, vespalib::GrowableByteBuffer& buf) const;
        virtual string decodeBucketSpace(document::ByteBuffer&) const;
    protected:
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    };
    class StatBucketReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class StatDocumentMessageFactory : public DocumentMessageFactory {
    protected:
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    };
    class StatDocumentReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class UpdateDocumentMessageFactory : public DocumentMessageFactory {
    protected:
        const document::DocumentTypeRepo &_repo;
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override {
            return decodeMessage<UpdateDocumentMessage>(this, buf);
        }

        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    public:
        void decodeInto(UpdateDocumentMessage & msg, document::ByteBuffer & buf) const;
        UpdateDocumentMessageFactory(const document::DocumentTypeRepo &r) : _repo(r) {}
    };
    class UpdateDocumentReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class VisitorInfoMessageFactory : public DocumentMessageFactory {
    protected:
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    };
    class VisitorInfoReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class WrongDistributionReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };
    class QueryResultMessageFactory : public DocumentMessageFactory {
    protected:
        DocumentMessage::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentMessage &msg, vespalib::GrowableByteBuffer &buf) const override;
    };
    class QueryResultReplyFactory : public DocumentReplyFactory {
    protected:
        DocumentReply::UP doDecode(document::ByteBuffer &buf) const override;
        bool doEncode(const DocumentReply &reply, vespalib::GrowableByteBuffer &buf) const override;
    };

    ///////////////////////////////////////////////////////////////////////////
    //
    // Utilities
    //
    ///////////////////////////////////////////////////////////////////////////

    /**
     * This is a complement for the vespalib::GrowableByteBuffer.putString() method.
     *
     * @param in The byte buffer to read from.
     * @return The decoded string.
     */
    static string decodeString(document::ByteBuffer &in)
    { return RoutableFactories42::decodeString(in); }

    /**
     * This is a complement for the vespalib::GrowableByteBuffer.putBoolean() method.
     *
     * @param in The byte buffer to read from.
     * @return The decoded bool.
     */
    static bool decodeBoolean(document::ByteBuffer &in)
    { return RoutableFactories42::decodeBoolean(in); }

    /**
     * Convenience method to decode a 32-bit int from the given byte buffer.
     *
     * @param in The byte buffer to read from.
     * @return The decoded int.
     */
    static int32_t decodeInt(document::ByteBuffer &in)
    { return RoutableFactories42::decodeInt(in); }

    /**
     * Convenience method to decode a 64-bit int from the given byte buffer.
     *
     * @param in The byte buffer to read from.
     * @return The decoded int.
     */
    static int64_t decodeLong(document::ByteBuffer &in)
    { return RoutableFactories42::decodeLong(in); }


    /**
     * Convenience method to decode a document id from the given byte buffer.
     *
     * @param in The byte buffer to read from.
     * @return The decoded document id.
     */
    static document::DocumentId decodeDocumentId(document::ByteBuffer &in)
    { return RoutableFactories42::decodeDocumentId(in); }

    /**
     * Convenience method to encode a document id to the given byte buffer.
     *
     * @param id  The document id to encode.
     * @param out The byte buffer to write to.
     */
    static void encodeDocumentId(const document::DocumentId &id,
                                 vespalib::GrowableByteBuffer &out)
    { return RoutableFactories42::encodeDocumentId(id, out); }
};

}
