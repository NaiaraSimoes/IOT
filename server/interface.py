import streamlit as st
import mysql.connector
from datetime import datetime, timedelta
import uuid  # Importando o módulo uuid para gerar classIndex único
import matplotlib.pyplot as plt
import pandas as pd

# Conexão com o banco de dados
smartClassRoomDB = mysql.connector.connect(
    host="localhost",
    user="phpmyadmin",
    password="ex3%ZhYLadS^@7",
    database="smartClassRoom"
)

def add_student():
    st.title("Add Student")

    student_name = st.text_input("Enter Student Name")
    student_uid = st.text_input("Enter Student UID")

    if st.button("Add Student"):
        if student_name and student_uid:
            try:
                cursor = smartClassRoomDB.cursor()
                query = "INSERT INTO students (studentName, studentUID) VALUES (%s, %s)"
                values = (student_name, student_uid)
                cursor.execute(query, values)
                smartClassRoomDB.commit()
                st.success("Student added successfully!")
            except mysql.connector.Error as error:
                st.error(f"Error occurred: {error}")
        else:
            st.warning("Please fill in all fields!")

def add_class():
    st.title("Add Class")
    col1, col2, col3 = st.columns(3)
    with col1:
        class_acronym = st.text_input("Enter Class Acronym")
    with col2:
        class_number = st.text_input("Enter Class Number")
    with col3:
        room = st.text_input("Enter Room")

    col1, col2 = st.columns(2)
    with col1:
        start_date = st.date_input("Enter Start Date")

    with col2:
        start_time = st.time_input("Enter Start Time")
    
    end_date = datetime.combine(start_date, start_time) + timedelta(hours=2)

    if st.button("Add Class"):
        if class_number and start_date and start_time and room and class_acronym:
            try:
                start_datetime = datetime.combine(start_date, start_time)
                class_index = str(uuid.uuid4())

                cursor = smartClassRoomDB.cursor()
                query = """
                INSERT INTO classes (classIndex, classNumber, startDate, endDate, room, classAcronym) 
                VALUES (%s, %s, %s, %s, %s, %s)
                """
                values = (class_index, class_number, start_datetime, end_date, room, class_acronym)
                cursor.execute(query, values)
                smartClassRoomDB.commit()

                st.success("Class added successfully!")
            except mysql.connector.Error as error:
                st.error(f"Error occurred: {error}")
        else:
            st.warning("Please fill in all fields!")

def add_student_to_class():
    st.title("Add Student to Class")

    # Obtendo a lista de todos os alunos
    cursor = smartClassRoomDB.cursor()
    cursor.execute("SELECT studentIndex, studentName FROM students")
    students = {row[1]: row[0] for row in cursor.fetchall()}
    selected_student = st.selectbox("Select Student:", list(students.keys()))

    # Obtendo a lista de todos os acrônimos das classes
    cursor.execute("SELECT DISTINCT classAcronym FROM classes")
    acronyms = [row[0] for row in cursor.fetchall()]
    selected_acronym = st.selectbox("Select Class Acronym:", acronyms)

    # Obtendo a lista de todos os números das classes para o acrônimo selecionado
    cursor.execute("SELECT classNumber FROM classes WHERE classAcronym = %s", (selected_acronym,))
    class_numbers = [row[0] for row in cursor.fetchall()]
    selected_class_number = st.selectbox("Select Class Number:", class_numbers)

    if st.button("Add Student to Room"):
        try:
            # Obtendo o studentIndex do aluno selecionado
            student_index = students[selected_student]

            # Obtendo o classIndex da classe selecionada
            cursor.execute("SELECT classIndex FROM classes WHERE classAcronym = %s AND classNumber = %s",
                           (selected_acronym, selected_class_number))
            class_index = cursor.fetchone()[0]

            # Inserindo na tabela associatedStudents
            cursor.execute("INSERT INTO associatedStudents (studentIndex, classIndex) VALUES (%s, %s)",
                           (student_index, class_index))
            smartClassRoomDB.commit()

            st.success("Student added to room successfully!")
        except mysql.connector.Error as error:
            st.error(f"Error occurred: {error}")

def update_student():
    st.subheader("Update Student")

    cursor = smartClassRoomDB.cursor()
    cursor.execute("SELECT * FROM students")
    students = cursor.fetchall()

    selected_student = st.selectbox("Select Student to Update:", [student[1] for student in students])

    new_name = st.text_input("Enter New Student Name")
    new_uid = st.text_input("Enter New Student UID")

    if st.button("Update Student"):
        if new_name and new_uid:
            try:
                query = "UPDATE students SET studentName = %s, studentUID = %s WHERE studentName = %s"
                values = (new_name, new_uid, selected_student)
                cursor.execute(query, values)
                smartClassRoomDB.commit()

                st.success("Student updated successfully!")
            except mysql.connector.Error as error:
                st.error(f"Error occurred: {error}")
        else:
            st.warning("Please fill in all fields!")

    with st.sidebar:
        if st.button("Delete Selected Student", key="delete_button"):
            try:
                query = "DELETE FROM students WHERE studentName = %s"
                cursor.execute(query, (selected_student,))
                smartClassRoomDB.commit()

                st.success("Student deleted successfully!")
            except mysql.connector.Error as error:
                st.error(f"Error occurred: {error}")

def update_class():
    st.subheader("Update Class")

    cursor = smartClassRoomDB.cursor()

    # Obter todos os acrônimos de classes distintos
    cursor.execute("SELECT DISTINCT classAcronym FROM classes")
    acronyms = [row[0] for row in cursor.fetchall()]
    selected_acronym = st.selectbox("Select Class Acronym:", acronyms)

    # Obter todos os números de classe distintos para o acrônimo selecionado
    cursor.execute("SELECT DISTINCT classNumber FROM classes WHERE classAcronym = %s", (selected_acronym,))
    class_numbers = [row[0] for row in cursor.fetchall()]
    selected_class_number = st.selectbox("Select Class Number:", class_numbers)

    # Obter todas as datas de início para o acrônimo e número de classe selecionados
    cursor.execute("SELECT startDate FROM classes WHERE classAcronym = %s AND classNumber = %s", (selected_acronym, selected_class_number))
    start_dates = [row[0] for row in cursor.fetchall()]

    selected_start_date = st.selectbox("Select Start Date:", start_dates)

    # Mostrar opções para atualização ou exclusão
    col1, col2 = st.columns(2)
    with col1:
        new_start_date = st.date_input("Enter New Start Date", value=selected_start_date, key="new_start_date")
    with col2:
        new_start_time = st.time_input("Enter New Start Time", value=selected_start_date, key="new_start_time")

    if st.button("Update Start Date and Time"):
        try:
            # Combine a data e a hora fornecidas em um único objeto datetime
            new_start_datetime = datetime.combine(new_start_date, new_start_time)
            query = """
            UPDATE classes 
            SET startDate = %s
            WHERE classAcronym = %s AND classNumber = %s AND startDate = %s
            """
            values = (new_start_datetime, selected_acronym, selected_class_number, selected_start_date)
            cursor.execute(query, values)
            smartClassRoomDB.commit()

            st.success("Start date and time updated successfully!")
        except mysql.connector.Error as error:
            st.error(f"Error occurred: {error}")

    with st.sidebar:
        if st.button("Delete Selected Class", key="delete_class_button"):
            try:
                query = "DELETE FROM classes WHERE classAcronym = %s AND classNumber = %s AND startDate = %s"
                cursor.execute(query, (selected_acronym, selected_class_number, selected_start_date))
                smartClassRoomDB.commit()

                st.success("Class deleted successfully!")
            except mysql.connector.Error as error:
                st.error(f"Error occurred: {error}")


def update_student_to_class():
    st.title("Update Student's Class")

    # Obtendo a lista de todos os alunos
    cursor = smartClassRoomDB.cursor()
    cursor.execute("SELECT studentIndex, studentName FROM students")
    students = {row[1]: row[0] for row in cursor.fetchall()}
    selected_student = st.selectbox("Select Student:", list(students.keys()))

    # Obtendo os acrônimos das classes em que o aluno está registrado
    cursor.execute("SELECT c.classAcronym, c.classNumber, c.classIndex FROM classes c JOIN associatedStudents a ON c.classIndex = a.classIndex WHERE a.studentIndex = %s", (students[selected_student],))
    class_info = cursor.fetchall()

    if class_info:
        selected_acronym = st.selectbox("Select Class Acronym:", [info[0] for info in class_info])
        selected_class_number = [info[1] for info in class_info if info[0] == selected_acronym][0]
        class_index = [info[2] for info in class_info if info[0] == selected_acronym][0]

        st.success(f"{selected_student} is registered in classes with acronym: {', '.join([info[0] for info in class_info])}")
        st.info(f"{selected_student} is registered in class number: {selected_class_number}")

        class_numbers = [info[1] for info in class_info if info[0] == selected_acronym]
        new_class_number = st.selectbox("Select New Class Number:", class_numbers)
        
        # Botão para deletar a inscrição
        if st.sidebar.button("Delete Selected Student's Class"):
            try:
                # Deletar a informação da tabela associatedStudents
                delete_query = "DELETE FROM associatedStudents WHERE studentIndex = %s AND classIndex = %s"
                cursor.execute(delete_query, (students[selected_student], class_index))
                smartClassRoomDB.commit()
                st.success("Registration deleted successfully!")
            except mysql.connector.Error as error:
                st.error(f"Error occurred: {error}")
    else:
        st.warning(f"{selected_student} is not registered in any classes.")



def view():
    st.title("View Data by Room Number")
    
    cursor = smartClassRoomDB.cursor()
    col1, col2 = st.columns(2)
    with col1:
        cursor.execute("SELECT DISTINCT classAcronym FROM classes")
        acronyms = [row[0] for row in cursor.fetchall()]
        selected_acronym = st.selectbox("Select Class Acronym:", acronyms)
    
    with col2:
        cursor.execute("SELECT DISTINCT classNumber FROM classes WHERE classAcronym = %s", (selected_acronym,))
        room_numbers = [row[0] for row in cursor.fetchall()]
        selected_number = st.selectbox("Select Class Number:", room_numbers)

    if selected_acronym and selected_number:
        cursor.execute("SELECT DISTINCT startDate FROM classes WHERE classAcronym = %s AND classNumber = %s", (selected_acronym, selected_number))
        start_dates = [row[0] for row in cursor.fetchall()]

        for start_date in start_dates:
            st.subheader(f"Students in Room {selected_number} on {start_date}:")

            cursor.execute("SELECT s.studentName FROM students s JOIN presentStudents ps ON s.studentIndex = ps.studentIndex WHERE ps.classIndex IN (SELECT classIndex FROM classes WHERE classAcronym = %s AND classNumber = %s AND startDate = %s)", (selected_acronym, selected_number, start_date))
            present_students = [row[0] for row in cursor.fetchall()]

            if present_students:
                st.write("Present Students:")
                st.write(present_students)
            else:
                st.write("No students found for this room and date.")

def view_student_by_room_number():
    st.title("View Data by Room Number")
    
    cursor = smartClassRoomDB.cursor()
    
    col1, col2 = st.columns(2)
    with col1:
        cursor.execute("SELECT DISTINCT classAcronym FROM classes")
        acronyms = [row[0] for row in cursor.fetchall()]
        selected_acronym = st.selectbox("Select Class Acronym:", acronyms)
    
    with col2:
        cursor.execute("SELECT DISTINCT classNumber FROM classes WHERE classAcronym = %s", (selected_acronym,))
        room_numbers = [row[0] for row in cursor.fetchall()]
        selected_number = st.selectbox("Select Class Number:", room_numbers)

    if selected_acronym and selected_number:
        cursor.execute("SELECT classIndex, startDate, endDate FROM classes WHERE classAcronym = %s AND classNumber = %s", (selected_acronym, selected_number))
        classes = cursor.fetchall()

        for class_ in classes:
            class_index, start_date, end_date = class_

            st.subheader(f"Class Times: {start_date} - {end_date}")

            cursor.execute("SELECT s.studentName FROM students s JOIN presentStudents ps ON s.studentIndex = ps.studentIndex WHERE ps.classIndex = %s", (class_index,))
            present_students = [row[0] for row in cursor.fetchall()]

            cursor.execute("SELECT s.studentName FROM students s WHERE s.studentIndex NOT IN (SELECT studentIndex FROM presentStudents WHERE classIndex = %s) AND s.studentIndex IN (SELECT studentIndex FROM associatedStudents WHERE classIndex = %s)", (class_index, class_index))
            absent_students = [row[0] for row in cursor.fetchall()]

            

            col1, col2 = st.columns(2)

            with col1:
                st.write("### Present Students")
                for student in present_students:
                    st.write(student)

            with col2:
                st.write("### Absent Students")
                for student in absent_students:
                    st.write(student)

def view_temp_by_room_number():
    st.title("View Temperature and Humidity by Room Number")

    smartClassRoomCursor = smartClassRoomDB.cursor()

    # Seleção do número da sala
    smartClassRoomCursor.execute("SELECT DISTINCT room FROM tempHumidity")
    rooms = [row[0] for row in smartClassRoomCursor.fetchall()]
    selected_room = st.selectbox("Select Room Number:", rooms)

    if selected_room:
        smartClassRoomCursor.execute("SELECT temperature, humidity, readingTimeDate FROM tempHumidity WHERE room = %s", (selected_room,))
        records = smartClassRoomCursor.fetchall()

        if records:
            st.subheader(f"Temperature and Humidity Readings for Room {selected_room}:")

            # Criando um DataFrame para facilitar a manipulação dos dados
            df = pd.DataFrame(records, columns=["Temperature (°C)", "Humidity (%)", "Reading Time"])
            df["Reading Time"] = pd.to_datetime(df["Reading Time"])

            col1, col2 = st.columns(2)
            with col1:
            # Plotando o gráfico de temperatura
                plt.figure(figsize=(10, 6))
                plt.plot(df["Reading Time"], df["Temperature (°C)"], label="Temperature (°C)", marker='o', color='red')
                plt.xlabel("Reading Time")
                plt.ylabel("Temperature (°C)")
                plt.title("Temperature Readings Over Time")
                plt.legend()
                plt.xticks(rotation=45)
                plt.tight_layout()

                # Exibindo o gráfico de temperatura no Streamlit
                st.subheader("Temperature Readings:")
                st.pyplot(plt)
            with col2:
                # Plotando o gráfico de umidade
                plt.figure(figsize=(10, 6))
                plt.plot(df["Reading Time"], df["Humidity (%)"], label="Humidity (%)", marker='o', color='blue')
                plt.xlabel("Reading Time")
                plt.ylabel("Humidity (%)")
                plt.title("Humidity Readings Over Time")
                plt.legend()
                plt.xticks(rotation=45)
                plt.tight_layout()

                # Exibindo o gráfico de umidade no Streamlit
                st.subheader("Humidity Readings:")
                st.pyplot(plt)

            # Exibindo os dados tabulares
          
        else:
            st.write("No temperature and humidity records found for this room.")



def main():
    st.set_page_config(page_title="Smart Classroom Admin", page_icon="🏫", layout="wide")

    st.markdown("""
        <style>
        .reportview-container {
            background: linear-gradient(45deg, #93a5cf, #e4efe9);
            color: #333;
        }
        .sidebar .sidebar-content {
            background: linear-gradient(45deg, #93a5cf, #e4efe9);
            color: #333;
        }
        .Widget>label {
            color: #333;
            font-weight: bold;
        }
        .stTextInput>div>div>input {
            background-color: #fff;
            color: #333;
        }
        .st-eb {
            background-color: #fff;
            border-radius: 12px;
        }
        </style>
        """, unsafe_allow_html=True)

    st.title("Smart Classroom Admin Interface")
    st.subheader("Welcome to the Smart Classroom Admin Interface!")

    # Adicione aqui as informações atuais que deseja exibir

    menu_options = ["Add Data", "Update Data", "View Data"]
    menu_choice = st.sidebar.selectbox("Select Option", menu_options)

    if menu_choice == "Add Data":
        add_data_options = ["Add Student", "Add Class", "Add Student to Class"]
        add_data_choice = st.sidebar.selectbox("Select Add Data Option", add_data_options)
        if add_data_choice == "Add Student":
            add_student()
        elif add_data_choice == "Add Class":
            add_class()
        elif add_data_choice == "Add Student to Class":
            add_student_to_class()

    elif menu_choice == "Update Data":
        update_data_options = ["Update Student", "Update Class", "Update Student to Class"]
        update_data_choice = st.sidebar.selectbox("Select Update Data Option", update_data_options)
        if update_data_choice == "Update Student":
            update_student()
        elif update_data_choice == "Update Class":
            update_class()
        elif update_data_choice == "Update Student to Class":
            update_student_to_class()

    elif menu_choice == "View Data":
        view_data_options = ["View Student", "View Temperature"]
        view_data_choice = st.sidebar.selectbox("Select View Data Option", view_data_options)
        if view_data_choice == "View Student":
            #view_student_by_room_number()
            view()
        elif view_data_choice == "View Temperature":
            view_temp_by_room_number()

if __name__ == "__main__":
    main()
