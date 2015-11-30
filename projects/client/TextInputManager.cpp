#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include "TextInputManager.hpp"
#include "Packet.hpp"
#include "App.hpp"
#include "PacketSender.hpp"

namespace NCC
{
    TextInputManager::TextInputManager()
    {
        mAllowInput = false;
        mLastKey = sf::Keyboard::Unknown;
    }

    void TextInputManager::Update()
    {
        sf::Event event;
        while(App::Instance().GetWindow()->pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
            {
                App::Instance().GetWindow()->close();
            }

            //poll event for keyboard input
            //Enabled after pressing enter.
            if (mAllowInput)
            {
                //check for special cases
                if (event.type == sf::Event::KeyPressed)
                {
                    switch (event.key.code)
                    {
                        case sf::Keyboard::Return:
                        {
                            if (mLastKey != sf::Keyboard::Return && mInputBuffer.size() > 0) //don't send 'empty' strings
                            {
                                //Process the input/message buffer
                                SendChatMessage();

                                //Disable input ready for next message
                                mAllowInput = false;

                                //Log keypress
                                mLastKey = sf::Keyboard::Return;

                                //Reset the input buffer
                                mInputBuffer = "";
                            }

                            else
                            {
                                mAllowInput = false;
                            }
                            return;
                        }

                        case sf::Keyboard::BackSpace:
                        {
                            //Delete last typed character when string is not empty.
                            if (mInputBuffer.size() > 0)
                                mInputBuffer.pop_back();
                            return;
                        }

                        case sf::Keyboard::Escape:
                        {
                            if (mLastKey != sf::Keyboard::Escape)
                            {
                                //Cancel text input.
                                mAllowInput = false;
                                mLastKey = sf::Keyboard::Escape;
                            }

                            return;
                        }

                        default:
                        {
                            //This is a normal character. Ignore it as it is handled during key release.
                            break;
                        }
                    }
                }

                //Reset last key on release
                else if (event.type == sf::Event::KeyReleased && event.key.code == mLastKey)
                {
                    mLastKey = sf::Keyboard::Unknown;
                    return;
                }

                //Append new character to message
                else if (event.type == sf::Event::TextEntered)
                {
                    // Handle ASCII characters only, skipping backspace and delete
                    if (event.text.unicode > 31 && event.text.unicode < 127 && mInputBuffer.size() < mMaxMessageLength)
                    {
                        mInputBuffer += static_cast<char>(event.text.unicode);
                    }
                }
            }

            //Check to see if we are enabling input
            else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return)
            {
                mAllowInput = true;
                mLastKey = sf::Keyboard::Return;
            }
        }
    }

    void TextInputManager::SendChatMessage()
    {
        //Create our packet, and set its type.
        PacketMessage messagePacket = PacketMessage();
        messagePacket.type = Packet::MESSAGE;

        //Set our length value so we know how big it is when getting our string back.
        messagePacket.length = mInputBuffer.length();

        //Initialize our packet char array to copy our message.
        //HACK - Possible memory leak.
        *messagePacket.message = *new char[mInputBuffer.size() + 1];

        //Null Terminator
        messagePacket.message[mInputBuffer.size()] = 0;

        //Copy the message into our char array.
        memcpy(messagePacket.message, mInputBuffer.c_str(), mInputBuffer.size());

        //Send our packet to the client.
        App::Instance().GetPacketSender()->AddPacketData(reinterpret_cast<char*>(&messagePacket), sizeof(messagePacket));
    }
}
