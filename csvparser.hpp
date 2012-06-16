/************************************************
 * CSVParser
 *                            tyabuta 2012.06.17
 ************************************************/

#pragma once

#include <iostream>
#include <vector>
#include <string>

/*
 * CSV�t�@�C�����Z����ɕ��͂���N���X
 * �_�u���N�H�[�e�[�V�����ɂ��݂͂�A�r���̉��s�ɂ��Ή����Ă��܂��B
 * �J���}��؂�Ή��� typedef ���ꂽ CSVParser ������܂����A
 * basic_csvparser<'\t'> �ŃC���X�^���X���쐬�����TAB��؂�ɑΉ��ł��܂��B
 */
template <int separate_ = ','>
class basic_csvparser{
public:
    typedef std::vector<std::string> Row;
    typedef std::vector<Row>         Cells;

private:
    enum State {
        STATE_FIRST,
        STATE_CELL_BEGIN,
        STATE_IN_CELL,
        STATE_IN_QUOTE,
        STATE_SECOND_QUOTE,
        STATE_EOF,
    };

    State       state_; // ��͒��ɕω����Ă�����ԃt���O
    std::string str_;   // 1Cell���̕�����o�b�t�@
    Cells*      data_;  // vector�z��̃|�C���^

    unsigned string_cursor_;
    unsigned col_cursor_;
    unsigned row_cursor_;

public:
    void parse(const std::string& csv_path, Cells& ref_data){
        // ������
        data_  = &ref_data;
        state_ = STATE_FIRST;

        // CSV�t�@�C���̃p�[�X�J�n
        FILE* f;
        fopen_s(&f, csv_path.c_str(), "r");
        if (f){
            while (1){
                char c = fgetc(f);
                put(c);
                if (c == EOF) break;
            } 
            fclose(f);
        }
    }

    // �X�g���[���ɏo�͂���B
    void output(std::ostream& out, const Cells& data){
        int row_counter = 0;
        for (auto r=data.begin(); r!=data.end(); r++, row_counter++){
            out << row_counter << ": ";

            for (auto c=r->begin(); c!=r->end(); c++){
                out << *c << " | ";
            }
            out << endl;
        }
    }

private:
    void put(char c){
        if (state_ == STATE_FIRST){
            str_.clear();
            string_cursor_ = 0;
            col_cursor_ = 0;
            row_cursor_ = 0;
            data_->clear();

            state_ = STATE_CELL_BEGIN;
        } 

        switch (state_){
        case STATE_CELL_BEGIN:
            seq_begin(c);
            break;
        case STATE_IN_CELL:
            seq_in_cell(c);
            break;
        case STATE_IN_QUOTE:
            seq_in_quote(c);
            break;
        case STATE_SECOND_QUOTE:
            seq_second_quote(c);
            break;
        }

        if (state_ == STATE_EOF){
            finish_row();
        }
    }



    void set_char(char c){
        str_ += c;
        string_cursor_++; // ������J�[�\����i�߂�B
    }

    void finish_cell(){
        (*data_)[row_cursor_][col_cursor_] = str_;
        str_.clear();
        string_cursor_ = 0;
        col_cursor_++;
    }

    void finish_row(){
        finish_cell();
        col_cursor_ = 0;
        row_cursor_++;
    }





    /*-------------------------------------------
      Sequence
    -------------------------------------------*/

    void seq_begin(char c){
        // Row�J�[�\�����ړ����Ă�����A�s��ǉ��B
        if (row_cursor_ >= data_->size()){
            data_->push_back(Row());    
        }

        // Column�J�[�\�����ړ����Ă�����A���ǉ��B
        if (col_cursor_ >= (*data_)[row_cursor_].size()){
            (*data_)[row_cursor_].push_back(std::string());
        }


        switch (c){
        case separate_: // ��؂蕶���Ȃ�A�v�f�m��B
            finish_cell();
            return;

        case '\n': // ���s�Ȃ�s�m��B
            finish_row();
            return;

        case EOF:
            state_ = STATE_EOF;
            return;

        case '\"':
            state_ = STATE_IN_QUOTE;
            return;
        }

        set_char(c);
        state_ = STATE_IN_CELL;
    }


    void seq_in_cell(char c){
        switch (c){
        case separate_: // ��؂蕶���Ȃ�A�v�f�m��B
            finish_cell();
            state_ = STATE_CELL_BEGIN;
            return;

        case '\n': // ���s�Ȃ�s�m��B
            finish_row();
            state_ = STATE_CELL_BEGIN;
            return;

        case EOF:
            state_ = STATE_EOF;
            return;
        }

        set_char(c);
    }

    void seq_in_quote(char c){
        switch (c){
        case EOF:
            state_ = STATE_EOF;
            return;

        case '\"':
            state_ = STATE_SECOND_QUOTE;
            return;
        }

        set_char(c);
    }

    void seq_second_quote(char c){
        switch (c){
        case separate_: // ��؂蕶���Ȃ�A�v�f�m��B
            finish_cell();
            state_ = STATE_CELL_BEGIN;
            return;

        case '\n': // ���s�Ȃ�s�m��B
            finish_row();
            state_ = STATE_CELL_BEGIN;
            return;

        case EOF:
            state_ = STATE_EOF;
            return;
        }

        set_char(c);
        state_ = STATE_IN_QUOTE;
    }



};

typedef basic_csvparser<','> CSVParser;

